#include "uart/uart.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//  #    #    #    #####  #######                                                 
//  #    #   # #   #    #    #                                                    
//  #    #  #   #  #    #    #                                                    
//  #    # #     # #####     #                                                    
//  #    # ####### #  #      #                                                    
//  #    # #     # #   #     #                                                    
//   ####  #     # #    #    #                                                    
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
#include "config/config_extn.hpp"
#include "uart/parity.hpp"
#include <algorithm>
#include <random>
namespace {
  const char* const MSGID{"/Doulos/Example/Uart"};
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
Uart_module::Uart_module // Constructor
( sc_module_name instance_name
, uint32_t       addr_clocks
, uint32_t       read_clocks
, uint32_t       write_clocks
)
: m_addr_clocks             { addr_clocks     }
, m_read_clocks             { read_clocks     }
, m_write_clocks            { write_clocks    }
, m_targ_peq                { this, &Uart_module::targ_peq_cb }
{
  SC_HAS_PROCESS( Uart_module );
  SC_THREAD( tx_thread );
  SC_THREAD( rx_thread );
  SC_METHOD( execute_transaction_process );
    sensitive << m_target_done_event;
    dont_initialize();
  targ_socket.register_b_transport        ( this, &Uart_module::b_transport );
  targ_socket.register_nb_transport_fw    ( this, &Uart_module::nb_transport_fw );
  targ_socket.register_transport_dbg      ( this, &Uart_module::transport_dbg );
  m_configuration.set( "name",         string(name())  );
  m_configuration.set( "kind",         string(kind())  );
  m_configuration.set( "object_ptr",   uintptr_t(this) );
  m_configuration.set( "addr_clocks",  addr_clocks     );
  m_configuration.set( "read_clocks",  read_clocks     );
  m_configuration.set( "write_clocks", write_clocks    );
  m_configuration.set( "coding_style", Style::AT       );
  INFO( ALWAYS, "Constructed " << name() << " with configuration:\n" << m_configuration );
}

//------------------------------------------------------------------------------
// Destructor
Uart_module::~Uart_module( void )
{
  INFO( ALWAYS, "Destroyed " << name() );
}

//------------------------------------------------------------------------------
void Uart_module::end_of_elaboration( void )
{
  // Validate connectivity
  if ( intrq_port.size() == 0 ) {
    REPORT( WARNING, "Interrupt not connected." );
  }
  if ( txslow_port.size() > 1 ) {
    REPORT( ERROR, "Connectivity - txslow_port may only be bound once" );
  }
  if ( txfast_port.size() > 1 ) {
    REPORT( ERROR, "Connectivity - txslow_port may only be bound once" );
  }
  if ( txslow_port.size() > 0 and txfast_port.size() > 0 ) {
    REPORT( ERROR, "Connectivity - choose only one of txfast_port or txslow_port" );
  }
  if ( txslow_port.size() == 0 and txfast_port.size() == 0 ) {
    INFO( ALWAYS, "Using virtual port for transmitter on " << name() );
    m_txspeed = Speed::VIRTUAL;
  }
  else if( txfast_port.size() == 1 ) {
    INFO( ALWAYS, "Using fast port for transmitter on " << name() );
    m_txspeed = Speed::FAST;
  }
  else {
    INFO( ALWAYS, "Using slow port for transmitter on " << name() );
    m_txspeed = Speed::SLOW;
  }
  if ( rxslow_port.size() > 1 ) {
    REPORT( ERROR, "Connectivity - rxslow_port may only be bound once" );
  }
  if ( rxfast_port.size() > 1 ) {
    REPORT( ERROR, "Connectivity - rxslow_port may only be bound once" );
  }
  if ( rxslow_port.size() > 0 and rxfast_port.size() > 0 ) {
    REPORT( ERROR, "Connectivity - choose only one of rxfast_port or rxslow_port" );
  }
  if ( rxslow_port.size() == 0 and rxfast_port.size() == 0 ) {
    INFO( ALWAYS, "Using virtual port for receiver on " << name() );
    m_rxspeed = Speed::VIRTUAL;
  }
  else if( rxfast_port.size() == 1 ) {
    INFO( ALWAYS, "Using fast port for receiver on " << name() );
    m_rxspeed = Speed::FAST;
  }
  else {
    INFO( ALWAYS, "Using slow port for receiver on " << name() );
    m_rxspeed = Speed::SLOW;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Uart_module::b_transport
( tlm_payload_t& trans
, sc_time& delay
)
{

  Depth_t len = trans.get_data_length();

  if ( not payload_is_ok( trans, len, Style::LT ) ) {
    return;
  }
  transport( trans, delay );
}

//------------------------------------------------------------------------------
Depth_t
Uart_module::transport_dbg
( tlm_payload_t& trans
)
{
  INFO( DEBUG, "Executing " << name() << "." << __func__ << "::transport_dbg" );
  bool config_only{ configure( trans ) };

  if ( config_only ) {
    INFO( DEBUG, "config_only" );
    trans.set_response_status( TLM_OK_RESPONSE );
    return 0;
  }

  // Allow for length beyond end by truncating
  Addr_t  adr = trans.get_address();
  Depth_t len = trans.get_data_length();

  if ( ( adr + len - 1 ) > m_target_size ) {
    len -= ( adr + len - 1 ) - m_target_size;
  }

  if ( not payload_is_ok( trans, len, Style::LT ) ) {
    return 0;
  }

  uint8_t* ptr = trans.get_data_ptr();
  uint8_t* reg = m_register_vec.data();
  if ( trans.is_write() ) {
    memcpy( reg + adr, ptr, len );
  }
  else if ( trans.is_read() ) {
    memcpy( ptr, reg + adr, len );
  }
  else {
    len = 0;
  }
  return len;
}

//------------------------------------------------------------------------------
// Return true if configuration is all that is needed
bool Uart_module::configure( tlm_payload_t& trans )
{
  Config_extn* extn{trans.get_extension<Config_extn>()};

  if ( extn != nullptr ) {
    INFO( DEBUG, "Configuring " << name() );

    if ( extn->configuration.empty() ) {
      NOINFO( DEBUG, "Sending configuration:\n" << m_configuration );
      extn->configuration = m_configuration;
    }
    else {
      m_configuration.update( extn->configuration );
      // Update local copies
      extn->configuration.get( "target_size",  m_target_size  );
      extn->configuration.get( "addr_clocks", m_addr_clocks    );
      extn->configuration.get( "read_clocks", m_read_clocks    );
      extn->configuration.get( "write_clocks", m_write_clocks );
      INFO( DEBUG, "Updated configuration " << m_configuration );
    }
  }

  trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
  return trans.get_command() == TLM_IGNORE_COMMAND;
}

//------------------------------------------------------------------------------
void Uart_module::execute_transaction( tlm_payload_t& trans )
{
  Depth_t len = trans.get_data_length();
  if ( not payload_is_ok( trans, len, Style::AT ) ) {
    return;
  }
  sc_time delay( SC_ZERO_TIME );
  transport( trans, delay );
}

//------------------------------------------------------------------------------
tlm_sync_enum
Uart_module::nb_transport_fw
( tlm_payload_t& trans
, tlm_phase& phase
, sc_time& delay
)
{

  // Queue the transaction until the annotated time has elapsed
  m_targ_peq.notify( trans, phase, delay );
  return TLM_ACCEPTED;
}

////////////////////////////////////////////////////////////////////////////////
// Helpers

//------------------------------------------------------------------------------
void
Uart_module::targ_peq_cb
( tlm_payload_t& trans
, const tlm_phase_t& phase
)
{
  sc_time delay;

  switch ( phase ) {
    case BEGIN_REQ: {

    // Increment the transaction reference count
    trans.acquire();

      if ( m_transaction_in_progress != nullptr ) {
      send_end_req( trans );
      }
      else {
      // Put back-pressure on initiator by deferring END_REQ until pipeline is clear
      m_end_req_pending = &trans;
          }

    break;
    }

    case END_RESP: {
    // On receiving END_RESP, the target can release the transaction
    // and allow other pending transactions to proceed

      if ( not m_response_in_progress ) {
      REPORT( FATAL, "Illegal transaction phase END_RESP received by target" );
      }

    // Flag must only be cleared when END_RESP is sent
    m_transaction_in_progress = nullptr;

    // Target itself is now clear to issue the next BEGIN_RESP
    m_response_in_progress = false;

      if ( m_next_response_pending != nullptr ) {
      send_response( *m_next_response_pending );
      m_next_response_pending = nullptr;
    }

    // ... and to unblock the initiator by issuing END_REQ
      if ( m_end_req_pending != nullptr ) {
      send_end_req( *m_end_req_pending );
      m_end_req_pending = nullptr;
    }

    break;
    }

  case END_REQ:
    case BEGIN_RESP: {
    REPORT( FATAL, "Illegal transaction phase received by target" );
    break;
  }
  }//endswitch
}

//------------------------------------------------------------------------------
void
Uart_module::send_end_req( tlm_payload_t& trans )
{
  tlm_phase_t bw_phase;
  sc_time delay;

  // Queue the acceptance and the response with the appropriate latency
  bw_phase = END_REQ;
  delay = clk.period( 1 ); // Accept delay

  tlm_sync_enum status = targ_socket->nb_transport_bw( trans, bw_phase, delay );
  // Ignore return value; initiator cannot terminate transaction at this point

  // Queue internal event to mark beginning of response
  delay += clk.period( 1 ); // Latency
  m_target_done_event.notify( delay );

  sc_assert( m_transaction_in_progress == nullptr );
  m_transaction_in_progress = &trans;
}

//------------------------------------------------------------------------------
void
Uart_module::send_response( tlm_payload_t& trans )
{
  tlm_sync_enum status;
  tlm_phase_t   bw_phase;
  sc_time delay;

  m_response_in_progress = true;
  bw_phase = BEGIN_RESP;
  delay = SC_ZERO_TIME;
  status = targ_socket->nb_transport_bw( trans, bw_phase, delay );

  if ( status == TLM_UPDATED ) {
    // The timing annotation must be honored
    m_targ_peq.notify( trans, bw_phase, delay );
  }
  else if ( status == TLM_COMPLETED ) {
    // The initiator has terminated the transaction
    m_transaction_in_progress = nullptr;
    m_response_in_progress = false;
  }

  trans.release();
}

//------------------------------------------------------------------------------
// Method process that runs on target_done_event
void
Uart_module::execute_transaction_process( void )
{
  // Execute the read or write commands
  execute_transaction( *m_transaction_in_progress );

  // Target must honor BEGIN_RESP/END_RESP exclusion rule
  // i.e. must not send BEGIN_RESP until receiving previous END_RESP or BEGIN_REQ
  if ( m_response_in_progress ) {
    // Target allows only two transactions in-flight
    if ( m_next_response_pending != nullptr ) {
      REPORT( FATAL, "Attempt to have two pending responses in target" );
    }

    m_next_response_pending = m_transaction_in_progress;
  }
  else {
    send_response( *m_transaction_in_progress );
  }
}

//------------------------------------------------------------------------------
bool Uart_module::payload_is_ok
( tlm_payload_t& trans, Depth_t len, Style coding_style )
{
  tlm_command cmd = trans.get_command();
  Addr_t      adr = trans.get_address();
  uint8_t*    ptr = trans.get_data_ptr();
  uint8_t*    byt = trans.get_byte_enable_ptr();
  Depth_t     wid = trans.get_streaming_width();

  if ( ( adr + len ) >= m_target_size or ( adr & 3 ) != 0 ) {
    if ( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device or misaligned " << name() << " with address " << adr );
      trans.set_response_status( TLM_OK_RESPONSE );
    }
    else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }

    return false;
  }
  else if ( byt != 0 ) {
    if ( g_error_at_target ) {
      REPORT( ERROR, "Attempt to unsupported use byte enables " << name() << " with address " << adr );
      trans.set_response_status( TLM_OK_RESPONSE );
    }
    else {
      trans.set_response_status( TLM_BYTE_ENABLE_ERROR_RESPONSE );
    }

    return false;
  }
  else if ( ( ( len & 3 ) != 0 ) or ( coding_style == Style::AT and m_max_burst > 0 and len > m_max_burst ) ) {
    if ( g_error_at_target ) {
      REPORT( ERROR, "Attempt to burst " << len << " bytes to " << name() << " with address " << adr << " when max burst size is " << m_max_burst );
      trans.set_response_status( TLM_OK_RESPONSE );
    }
    else {
      trans.set_response_status( TLM_BURST_ERROR_RESPONSE );
    }

    return false;
  }
  else if ( wid < len ) { // No streaming
    if ( g_error_at_target ) {
      REPORT( ERROR, "Attempt to stream to " << name() << " with address " << adr );
      trans.set_response_status( TLM_OK_RESPONSE );
    }
    else {
      trans.set_response_status( TLM_GENERIC_ERROR_RESPONSE );
    }

    return false;
  }
  else {
    return true;
  }
}

//------------------------------------------------------------------------------
Depth_t
Uart_module::transport ( tlm_payload_t& trans, sc_time& delay)
{
  Addr_t     adr = trans.get_address();
  uint8_t*   ptr = trans.get_data_ptr();
  Depth_t    len = trans.get_data_length();
  Depth_t    sbw = targ_socket.get_bus_width() / 8;
  sc_assert( adr + len < m_target_size );
  uint8_t*   reg = m_register_vec.data();
  delay += clk.period( m_addr_clocks );

  INFO( DEBUG+1, "Transport to address " << HEX << adr << " in " << name() );
  if ( trans.is_write() ) {
    memcpy( reg + adr, ptr, len );
    delay += clk.period( m_write_clocks ) * ( ( len + sbw - 1 ) / sbw );
    write_actions( trans, delay );
  }
  else if ( trans.is_read() ) {
    read_actions( trans, delay );
    memcpy( ptr, reg + adr, len );
    delay += clk.period( m_read_clocks ) * ( ( len + sbw - 1 ) / sbw );
  }
  else {
    len = 0;
  }

  trans.set_response_status( TLM_OK_RESPONSE );
  return len;
}

////////////////////////////////////////////////////////////////////////////////
// Uart main actions
//------------------------------------------------------------------------------
void Uart_module::send( void )
{
  if ( m_txfifo.empty() ) return;
  switch( m_txspeed ) {
    //--------------------------------------------------------------------------
    case Speed::VIRTUAL:
    {
      m_txcount += m_txfifo.size();
      do {
        INFO( DEBUG, name() << " tx: " << int( m_txfifo.front() ) );
        m_txfifo.pop_front();
      } while( m_txfifo.empty() );
      break;
    }
    //--------------------------------------------------------------------------
    case Speed::FAST:
    case Speed::SLOW:
    {
      // Handled by tx_thread
      break;
    }
    //--------------------------------------------------------------------------
    default:
    {
      REPORT( FATAL, "Illegal txspeed setting" );
      break;
    }
  }
}//end Uart_module::send()

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//
//  ####### #     #        ####### #    # #####  #####    #    ####               
//     #     #   #            #    #    # #    # #       # #   #   #              
//     #      # #             #    #    # #    # #      #   #  #    #             
//     #       #              #    ###### #####  ##### #     # #    #             
//     #      # #             #    #    # #  #   #     ####### #    #             
//     #     #   #            #    #    # #   #  #     #     # #   #              
//     #    #     # ######    #    #    # #    # ##### #     # ####               
//
////////////////////////////////////////////////////////////////////////////////
void Uart_module::tx_thread( void )
{
  switch( m_txspeed ) {
    //--------------------------------------------------------------------------
    case Speed::VIRTUAL:
      for(;;) {
        NOT_YET_IMPLEMENTED();
      }
      break;
    //--------------------------------------------------------------------------
    case Speed::FAST:
    {
      for(;;) {
        NOT_YET_IMPLEMENTED();
      }
    }
    //--------------------------------------------------------------------------
    case Speed::SLOW:
    {
      for(;;) {
        NOT_YET_IMPLEMENTED();
      }
    }
    //--------------------------------------------------------------------------
    default:
    {
      REPORT( FATAL, "Illegal txspeed setting" );
      break;
    }
  }
  REPORT( FATAL, "Exited tx_thread" );
}//end Uart_module::tx_thread()

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//
//  #####  #     #        ####### #    # #####  #####    #    ####               
//  #    #  #   #            #    #    # #    # #       # #   #   #              
//  #    #   # #             #    #    # #    # #      #   #  #    #             
//  #####     #              #    ###### #####  ##### #     # #    #             
//  #  #     # #             #    #    # #  #   #     ####### #    #             
//  #   #   #   #            #    #    # #   #  #     #     # #   #              
//  #    # #     # ######    #    #    # #    # ##### #     # ####               
//
////////////////////////////////////////////////////////////////////////////////
void Uart_module::rx_thread( void )
{
  switch( m_rxspeed ) {
    //--------------------------------------------------------------------------
    case Speed::VIRTUAL:
      for(;;) {
        NOT_YET_IMPLEMENTED();
      }
      break;
    //--------------------------------------------------------------------------
    case Speed::FAST:
    {
      for(;;) {
        NOT_YET_IMPLEMENTED();
      }
    }
    //--------------------------------------------------------------------------
    case Speed::SLOW:
    {
      for(;;) {
        NOT_YET_IMPLEMENTED();
      }
    }
    //--------------------------------------------------------------------------
    default:
    {
      REPORT( FATAL, "Illegal rxspeed setting" );
      break;
    }
  }
  REPORT( FATAL, "Exited rx_thread" );
}//end Uart_module::rx_thread()

//------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////
//
//  #     #                              #                                        
//  #  #  #            #                # #           #                           
//  #  #  #        #   #               #   #          #    #                ####  
//  #  #  # # ###     ###   ###       #     #  ####  ###       ####  ####  #      
//  #  #  # ##    ##   #   #####      ####### #       #   ##  #    # #   #  ####  
//  #  #  # #      #   # # #          #     # #       # #  #  #    # #   #      # 
//   ## ##  #     ###   #   ###       #     #  ####    #  ###  ####  #   #  ####  
//
//////////////////////////////////////////////////////////////////////////////////
void Uart_module::write_actions( tlm_payload_t& trans, const sc_time& delay )
{
  // Make it easier to access data
  Uart_regs_t& reg{ *reinterpret_cast<Uart_regs_t*>( m_register_vec.data() ) };
  Addr_t address = trans.get_address();
  Addr_t reg_address = address & ~3ull; // Clear lower 4-bits (assumes word access)

  switch ( reg_address ) {
    case /*write*/ UART_TXCTRL_REG:
    {
      m_txctrl.configok = true;
      //txctrl.configok - read-only
      m_txctrl.fifoerr  = IS_NONZERO( UART_FIFOERR,   reg.txctrl );
      m_txctrl.dataerr  = IS_NONZERO( UART_DATAERR,   reg.txctrl );
      m_txctrl.running  = IS_NONZERO( UART_RUNNING,   reg.txctrl );
      m_txctrl.enadma   = IS_NONZERO( UART_ENADMA,    reg.txctrl );
      m_txctrl.enaintr  = IS_NONZERO( UART_ENAINTR,   reg.txctrl );
      m_txctrl.databits = IS_NONZERO( UART_DATABITS,  reg.txctrl );
      m_txctrl.stopbits = IS_NONZERO( UART_STOPBITS,  reg.txctrl );
      m_txctrl.parity   = IS_NONZERO( UART_PARITY,    reg.txctrl );
      m_txctrl.odd      = IS_NONZERO( UART_ODD,       reg.txctrl );
      //txctrl.fifoused - Read-only
      //txctrl.fifoused - Read-only
      m_txctrl.baudrate = GET_FIELD(UART_BAUDRATE, reg.txctrl);
      m_txbits = 8 - (m_txctrl.databits?8:7) + (m_txctrl.parity?0:1) + (m_txctrl.stopbits?1:2);
      if ( m_txctrl.baudrate < sizeof( m_baud ) ) {
        m_uart.txclocks = sc_freq( m_baud[ m_txctrl.baudrate ], SC_HZ ) * clk.period() * m_txbits;
      }
      else if ( m_txctrl.running ) {
        m_txctrl.configok = false;
        if ( intrq_port.size() == 1 ) {
          intrq_port->notify( name() );
        }
      }
      if ( m_txctrl.configok ) {
        if ( m_txctrl.running and not IS_NONZERO( UART_RUNNING, m_uart.txctrl ) ) {
          // Start
          if ( not m_txfifo.empty() ) {
            send();
          }
        } else if ( IS_NONZERO( UART_RUNNING, m_uart.txctrl ) and not m_txctrl.running ) {
          // Stop
          m_txevent.notify();
        }
      }
      m_uart.txctrl = reg.txctrl;
      break;
    }
    case /*write*/ UART_TXDMAP_REG:
    {
      m_uart.txdmap = reg.txdmap;
      break;
    }
    case /*write*/ UART_TXDMAC_REG:
    {
      m_uart.txdmac = reg.txdmac;
      break;
    }
    case /*write*/ UART_TXDATA_REG:
    {
      m_uart.txdata = m_register_vec[UART_TXDATA_REG];
      if( m_txfifo.size() == m_txfifo_capacity ) {
        ++m_txdropped;
      } else {
        m_uart.txdata &= ~( m_txctrl.databits ? 0x7F : 0xFF );
        m_txfifo.push_back( m_uart.txdata );
        send();
      }
      break;
    }
    case /*write*/ UART_RXCTRL_REG:
    {
      m_rxctrl.configok = true;
      //rxctrl.configok - read-only
      m_rxctrl.fifoerr  = IS_NONZERO( UART_FIFOERR,   reg.rxctrl );
      m_rxctrl.dataerr  = IS_NONZERO( UART_DATAERR,   reg.rxctrl );
      m_rxctrl.running  = IS_NONZERO( UART_RUNNING,   reg.rxctrl );
      m_rxctrl.enadma   = IS_NONZERO( UART_ENADMA,    reg.rxctrl );
      m_rxctrl.enaintr  = IS_NONZERO( UART_ENAINTR,   reg.rxctrl );
      m_rxctrl.databits = IS_NONZERO( UART_DATABITS,  reg.rxctrl );
      m_rxctrl.stopbits = IS_NONZERO( UART_STOPBITS,  reg.rxctrl );
      m_rxctrl.parity   = IS_NONZERO( UART_PARITY,    reg.rxctrl );
      m_rxctrl.odd      = IS_NONZERO( UART_ODD,       reg.rxctrl );
      //rxctrl.fifoused - Read-only
      //rxctrl.fifoused - Read-only
      m_rxctrl.baudrate = GET_FIELD(UART_BAUDRATE, reg.rxctrl);
      m_rxbits = 8 - (m_rxctrl.databits?8:7) + (m_rxctrl.parity?0:1) + (m_rxctrl.stopbits?1:2);
      if ( m_rxctrl.baudrate < sizeof( m_baud ) ) {
        m_uart.rxclocks = sc_freq( m_baud[ m_rxctrl.baudrate ], SC_HZ ) * clk.period() * m_rxbits;
      }
      else if ( m_rxctrl.running ) {
        m_rxctrl.configok = false;
        if ( intrq_port.size() == 1 ) {
          intrq_port->notify( name() );
        }
      }
      if ( m_rxctrl.configok ) {
        if ( m_rxctrl.running and not IS_NONZERO( UART_RUNNING, m_uart.rxctrl ) ) {
          // Start
          m_rxevent.notify();
        } else if ( IS_NONZERO( UART_RUNNING, m_uart.rxctrl ) and not m_rxctrl.running ) {
          // Stop
          m_rxevent.notify();
        }
      }
      m_uart.rxctrl = reg.rxctrl;
      break;
    }
    case /*write*/ UART_RXDMAP_REG:
    {
      m_uart.rxdmap = reg.rxdmap;
      break;
    }
    case /*write*/ UART_RXDMAC_REG:
    {
      m_uart.rxdmac = reg.rxdmac;
      break;
    }
    case /*write*/ UART_RXDATA_REG:
    {
      REPORT( WARNING, "Attempt to write read-data ignored\n" );
      break;
    }
    case /*write*/ UART_TXCLOCKS_REG: // Read-only
    {
      REPORT( WARNING, "Attempt to write read-only txclocks chicken bits ignored\n" );
      break;
    }
    case /*write*/ UART_RXCLOCKS_REG: // Read-only
    {
      REPORT( WARNING, "Attempt to write read-only rxclocks chicken bits ignored\n" );
      break;
    }
    case /*write*/ UART_MISC_REG:
    {
      SET_FIELD( LE_BYTE0,   m_uart.misc, reg.misc  );
      break;
    }
    case /*write*/ UART_FIFOSTATS_REG: // Read-only
    {
      REPORT( WARNING, "Attempt to write read-only fifostats chicken bits ignored\n" );
      break;
    }
    default:
    {
      REPORT( ERROR, "Attempt to write illegal address" );
      break;
    }
  }
}

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//
//  #####                             #                                          
//  #    #                  #        # #           #                             
//  #    #                  #       #   #          #    #                ####    
//  #####   ###    ###      #      #     #  ####  ###       ####  ####  #        
//  #  #   #####  #   #  ####      ####### #       #   ##  #    # #   #  ####    
//  #   #  #      #  ## #   #      #     # #       # #  #  #    # #   #      #   
//  #    #  ###    ## #  ####      #     #  ####    #  ###  ####  #   #  ####    
//
////////////////////////////////////////////////////////////////////////////////
void Uart_module::read_actions( tlm_payload_t& trans, const sc_time& delay )
{
  // Make it easier to access data
  Uart_regs_t& reg{ *reinterpret_cast<Uart_regs_t*>( m_register_vec.data() ) };
  Addr_t address = trans.get_address();
  Addr_t reg_address = address & ~3ull; // Clear lower 4-bits (assumes word access)

  switch ( reg_address ) {
    case /*read*/ UART_TXCTRL_REG:
    {
      m_txctrl.depthend = m_txfifo.size() == m_txfifo_capacity;
      SET_FIELD( UART_CONFIGOK,   reg.txctrl, m_txctrl.configok );
      SET_FIELD( UART_FIFOERR,    reg.txctrl, m_txctrl.fifoerr  );
      SET_FIELD( UART_DATAERR,    reg.txctrl, m_txctrl.dataerr  );
      SET_FIELD( UART_RUNNING,    reg.txctrl, m_txctrl.running  );
      SET_FIELD( UART_ENADMA,     reg.txctrl, m_txctrl.enadma   );
      SET_FIELD( UART_ENAINTR,    reg.txctrl, m_txctrl.enaintr  );
      SET_FIELD( UART_DATABITS,   reg.txctrl, m_txctrl.databits );
      SET_FIELD( UART_STOPBITS,   reg.txctrl, m_txctrl.stopbits );
      SET_FIELD( UART_PARITY,     reg.txctrl, m_txctrl.parity   );
      SET_FIELD( UART_ODD,        reg.txctrl, m_txctrl.odd      );
      SET_FIELD( UART_DEPTHEND,   reg.txctrl, m_txctrl.depthend );
      SET_FIELD( UART_FIFOUSED,   reg.txctrl, m_txctrl.fifoused );
      SET_FIELD( UART_BAUDRATE,   reg.txctrl, m_txctrl.baudrate );
      break;
    }
    case /*read*/ UART_TXDMAP_REG:
    {
      reg.txdmap = m_uart.txdmap;
      break;
    }
    case /*read*/ UART_TXDMAC_REG:
    {
      reg.txdmac = m_uart.txdmac;
      break;
    }
    case /*read*/ UART_TXDATA_REG:
    {
      REPORT( WARNING, "Attempt to read write-data ignored\n" );
      reg.txdata = 0;
      m_register_vec[UART_TXDATA_REG] = uint8_t('\0xFF');
      break;
    }
    case /*read*/ UART_RXCTRL_REG:
    {
      m_rxctrl.depthend = m_rxfifo.size() == 0;
      SET_FIELD( UART_CONFIGOK,   reg.rxctrl, m_rxctrl.configok );
      SET_FIELD( UART_FIFOERR,    reg.rxctrl, m_rxctrl.fifoerr  );
      SET_FIELD( UART_DATAERR,    reg.rxctrl, m_rxctrl.dataerr  );
      SET_FIELD( UART_RUNNING,    reg.rxctrl, m_rxctrl.running  );
      SET_FIELD( UART_ENADMA,     reg.rxctrl, m_rxctrl.enadma   );
      SET_FIELD( UART_ENAINTR,    reg.rxctrl, m_rxctrl.enaintr  );
      SET_FIELD( UART_DATABITS,   reg.rxctrl, m_rxctrl.databits );
      SET_FIELD( UART_STOPBITS,   reg.rxctrl, m_rxctrl.stopbits );
      SET_FIELD( UART_PARITY,     reg.rxctrl, m_rxctrl.parity   );
      SET_FIELD( UART_ODD,        reg.rxctrl, m_rxctrl.odd      );
      SET_FIELD( UART_DEPTHEND,   reg.rxctrl, m_rxctrl.depthend );
      SET_FIELD( UART_FIFOUSED,   reg.rxctrl, m_rxctrl.fifoused );
      SET_FIELD( UART_BAUDRATE,   reg.rxctrl, m_rxctrl.baudrate );
      break;
    }
    case /*read*/ UART_RXDMAP_REG:
    {
      reg.rxdmap = m_uart.rxdmap;
      break;
    }
    case /*read*/ UART_RXDMAC_REG:
    {
      reg.rxdmac = m_uart.rxdmac;
      break;
    }
    case /*read*/ UART_RXDATA_REG:
    reg.rxdata = 0;
    {
      if ( not m_rxfifo.empty() ) {
        m_register_vec[ UART_RXDATA_REG ] = m_rxfifo.front();
        m_rxfifo.pop_front();
        ++m_rxcount;
      }
      else {
        m_register_vec[ UART_RXDATA_REG ] = uint8_t('\0xFF');
      }
      break;
    }
    case /*read*/ UART_TXCLOCKS_REG: // Read-only
    {
      reg.txclocks = m_uart.txclocks;
      break;
    }
    case /*read*/ UART_RXCLOCKS_REG: // Read-only
    {
      reg.rxclocks = m_uart.rxclocks;
      break;
    }
    case /*read*/ UART_MISC_REG: // Read-only
    {
      // data_bits + parity_bits + stop_bits
      SET_FIELD( UART_RXBITS,   reg.misc, m_rxbits );
      SET_FIELD( UART_TXBITS,   reg.misc, m_txbits );
      bool    rxselect = IS_NONZERO( UART_RXSELECT, reg.misc );
      uint8_t peekaddr = GET_FIELD( UART_PEEKADDR, reg.misc );
      uint8_t peekdata = uint8_t('\0xFF');
      if ( rxselect and peekaddr < m_rxfifo_capacity ) {
        peekdata = m_rxfifo[ peekaddr ];
      }
      else if ( peekaddr < m_txfifo_capacity ) {
        peekdata = m_txfifo[ peekaddr ];
      }
      SET_FIELD( UART_PEEKDATA, reg.misc, peekdata  );
      break;
    }
    case /*read*/ UART_FIFOSTATS_REG: // Read-only
    {
      NOT_YET_IMPLEMENTED();
      break;
    }
    default:
    {
      REPORT( ERROR, "Attempt to write illegal address" );
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END uart.cpp @(#)$Id$
