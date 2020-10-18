#include "gpio/gpio.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//   ####  #####  ###  ####                                                      
//  #    # #    #  #  #    #                                                     
//  #      #    #  #  #    #                                                     
//  #  ### #####   #  #    #                                                     
//  #    # #       #  #    #                                                     
//  #    # #       #  #    #                                                     
//   ####  #      ###  ####                                                      
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
#include "config/config_extn.hpp"
#include <algorithm>
#include <random>
namespace {
  const char* const MSGID{"/Doulos/Example/Gpio"};
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
Gpio_module::Gpio_module // Constructor
( sc_module_name instance_name
, uint32_t       addr_clocks
, uint32_t       read_clocks
, uint32_t       write_clocks
)
: m_addr_clocks             { addr_clocks     }
, m_read_clocks             { read_clocks     }
, m_write_clocks            { write_clocks    }
, m_targ_peq                { this, &Gpio_module::targ_peq_cb }
{
  gpio_xport.bind( gpio_sig );
  SC_HAS_PROCESS( Gpio_module );
  SC_THREAD( gpio_thread );
  SC_METHOD( execute_transaction_process );
    sensitive << m_target_done_event;
    dont_initialize();
  targ_socket.register_b_transport        ( this, &Gpio_module::b_transport );
  targ_socket.register_nb_transport_fw    ( this, &Gpio_module::nb_transport_fw );
  targ_socket.register_transport_dbg      ( this, &Gpio_module::transport_dbg );
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
Gpio_module::~Gpio_module( void )
{
  INFO( ALWAYS, "Destroyed " << name() );
}

//------------------------------------------------------------------------------
void Gpio_module::end_of_elaboration( void )
{
}

//------------------------------------------------------------------------------
void Gpio_module::start_of_simulation( void )
{
   for( int pin=0; pin<PINS; ++pin ) {
     gpio_sig[ pin ].write( SC_LOGIC_Z );
     m_inp[ pin ] = SC_LOGIC_X;
     m_out[ pin ] = SC_LOGIC_Z;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Gpio_module::b_transport
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
Gpio_module::transport_dbg
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
bool Gpio_module::configure( tlm_payload_t& trans )
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
void Gpio_module::execute_transaction( tlm_payload_t& trans )
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
Gpio_module::nb_transport_fw
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
Gpio_module::targ_peq_cb
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
Gpio_module::send_end_req( tlm_payload_t& trans )
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

  assert( m_transaction_in_progress == nullptr );
  m_transaction_in_progress = &trans;
}

//------------------------------------------------------------------------------
void
Gpio_module::send_response( tlm_payload_t& trans )
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
Gpio_module::execute_transaction_process( void )
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
bool Gpio_module::payload_is_ok
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
Gpio_module::transport ( tlm_payload_t& trans, sc_time& delay)
{
  Addr_t     adr = trans.get_address();
  uint8_t*   ptr = trans.get_data_ptr();
  Depth_t    len = trans.get_data_length();
  Depth_t    sbw = targ_socket.get_bus_width() / 8;
  sc_assert( adr + len < m_target_size );
  uint8_t*   reg = m_register_vec.data();
  delay += clk.period( m_addr_clocks );

  INFO( DEBUG+1, "Transport to address " << STREAM_HEX << adr << " in " << name() );
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
// Gpio main actions
//------------------------------------------------------------------------------
void Gpio_module::gpio_thread( void )
{
  sc_event_or_list port_changed;
  for( auto& sig : gpio_sig ) {
    port_changed |= sig.default_event();
  }
  for(;;) {
    wait( port_changed );
    // Move to the next rising edge of the clock (better emulates actual behavior)
    clk.wait_posedge();
    sc_lv<PINS> datain;
    bool interrupt = false; // assume not
    uint64_t bit = 1;
    for( int pin=0; pin<PINS; ++pin, bit<<=1 ) {
      // Is it an input? If not, move on.
      if( (m_reg.pindirn & bit) == 0 ) continue; //< 0 => output, else input
      // Read the value
      sc_logic pindata = gpio_sig[ pin ].read();
      // Is pull-logic in effect? If so, adjust value accordingly.
      if( (pindata == SC_LOGIC_Z) and (m_reg.pinpull & bit) ) {
        pindata = (m_reg.pulldir & bit) ? SC_LOGIC_1 : SC_LOGIC_0;
      }
      // Was it unknown? If so, set it to the opposite of current value
      else if( pindata == SC_LOGIC_X ) {
        pindata = ~ m_inp[ pin ];
      }
      // Did it change? If not, move on
      if( pindata == m_inp[pin] ) continue;
      bool changed = false; // assume not observed
      // It changed, so was it rising or falling?
      if( (pindata == SC_LOGIC_1) and (m_reg.pinrise & bit) ) {
        changed = true;
        m_reg.datachg |= bit;
      }
      else if( (pindata == SC_LOGIC_0) and (m_reg.pinfall & bit) ) {
        changed = true;
        m_reg.datachg |= bit;
      }
      // If it changed, was it enabled to interrupt?
      if( changed and (m_reg.pinintr & bit) ) {
        interrupt = true;
      }
    }//endfor
    // Interrupt if needed and port is connected
    if( interrupt and (intrq_port.size() == 1) ) {
      intrq_port->notify( name() );
    }
  }
}//end Gpio_module::gpio_thread()

//------------------------------------------------------------------------------
void Gpio_module::write_actions( tlm_payload_t& trans, const sc_time& delay )
{
  // Make it easier to access data
  Gpio_regs_t& reg{ *reinterpret_cast<Gpio_regs_t*>( m_register_vec.data() ) };
  Addr_t address = trans.get_address();
  Addr_t reg_address = address & ~7ull; // Clear lower 3-bits

  std::bitset<PINS> new_output { 0 };
  std::bitset<PINS> new_input  { 0 };
  switch ( reg_address ) {
    case /*write*/ GPIO_PINDIRN_REG: // x0 0=out, 1=in (tri)
    {
      // Figure out what pins changed to outputs
      // - Inputs take care of themselves
      uint64_t bit = 1ull;
      for( int pin=0; pin<PINS; ++pin ) {
        if( ( m_reg.pindirn & bit ) and !( reg.pindirn & bit ) ) {
          new_output[ pin ] = true;
        }
        else if( !( m_reg.pindirn & bit ) and ( reg.pindirn & bit ) ) {
          new_input[ pin ] = true;
        }
      }
      m_reg.pindirn = reg.pindirn;
      break;
    }
    case /*write*/ GPIO_PININTR_REG: // x1 1=enabled to interrupt
    {
      m_reg.pinintr = reg.pinintr;
      break;
    }
    case /*write*/ GPIO_PINPULL_REG: // x2 1=pull
    {
      m_reg.pinpull = reg.pinpull;
      break;
    }
    case /*write*/ GPIO_PULLDIR_REG: // x3 0=down/1=up
    {
      m_reg.pulldir = reg.pulldir;
      break;
    }
    case /*write*/ GPIO_DATAINP_REG: // x4 =
    {
      // No effect
      break;
    }
    case /*write*/ GPIO_DATAOUT_REG: // x5 =
    {
      m_reg.dataout = reg.dataout;
      break;
    }
    case /*write*/ GPIO_DATASET_REG: // x6 |=
    {
      m_reg.dataset = reg.dataset;
      m_reg.dataout |= reg.dataset;
      break;
    }
    case /*write*/ GPIO_DATACLR_REG: // x7 &= ~
    {
      m_reg.dataclr = reg.dataclr;
      m_reg.dataout &= ~reg.dataclr;
      break;
    }
    case /*write*/ GPIO_DATAINV_REG: // x8 ^=
    {
      m_reg.datainv = reg.datainv;
      m_reg.dataout ^= reg.datainv;
      break;
    }
    case /*write*/ GPIO_DATACHG_REG: // x9 input change detected
    {
      // Only allowed to clear
      m_reg.datachg &= reg.datachg;
      break;
    }
    case /*write*/ GPIO_DATAENA_REG: // xA write 1's to clear changed bits
    {
      m_reg.dataena = reg.dataena;
      m_reg.datachg &= ~reg.dataena;
      break;
    }
    case /*write*/ GPIO_PINRISE_REG: // xB 1's indicate rising detection
    {
      m_reg.pinrise = reg.pinrise;
      break;
    }
    case /*write*/ GPIO_PINFALL_REG: // xC 1's indicate falling detection
    {
      m_reg.pinfall = reg.pinfall;
      break;
    }
    default: // ignore
    {
      break;
    }
  }//endswitch
  if( new_output.any() or (m_out != m_reg.dataout) ) {
    m_out =  m_reg.dataout;
    // Only update pins configured as outputs
    uint64_t bit = 1ull;
    for( int pin=0; pin<PINS; ++pin, bit<<=1 ) {
      // Is it an output?
      if( m_reg.pindirn & bit ) continue; // Non-zero => input
      // Has it changed? No need to output same data
      sc_logic value = ( m_reg.dataout & bit ) ? SC_LOGIC_1 : SC_LOGIC_0 ;
      if( gpio_sig[ pin ].read() == value ) continue;
      gpio_sig[ pin ].write( value );
    }//endfor
  }
  if( new_input.any() ) {
    uint64_t bit = 1ull;
    for( int pin=0; pin<PINS; ++pin, bit<<=1 ) {
      if( new_input[ pin ] ) gpio_sig[ pin ] = SC_LOGIC_Z;
    }//endfor
  }
}

//------------------------------------------------------------------------------
void Gpio_module::read_actions( tlm_payload_t& trans, const sc_time& delay )
{
  // Make it easier to access data
  Gpio_regs_t& reg{ *reinterpret_cast<Gpio_regs_t*>( m_register_vec.data() ) };
  Addr_t address = trans.get_address();
  Addr_t reg_address = address & ~7ull; // Clear lower 3-bits

  switch ( reg_address ) {
    case /*read*/ GPIO_PINDIRN_REG: // x0 0=out, 1=in (tri)
    {
      reg.pindirn = m_reg.pindirn;
      break;
    }
    case /*read*/ GPIO_PININTR_REG: // x1 1=enabled to interrupt
    {
      reg.pinintr = m_reg.pinintr;
      break;
    }
    case /*read*/ GPIO_PINPULL_REG: // x2 1=pull
    {
      reg.pinpull = m_reg.pinpull;
      break;
    }
    case /*read*/ GPIO_PULLDIR_REG: // x3 0=down/1=up
    {
      reg.pulldir = m_reg.pulldir;
      break;
    }
    case /*read*/ GPIO_DATAINP_REG: // x4 =
    {
      uint64_t bit = ( 1ull << (PINS-1) );
      m_reg.datainp = 0;
      for( int pin=PINS; bit!=0; bit >>= 1 ) {
        --pin;
        sc_logic v = gpio_sig[ pin ].read();
        // If pulled, adjust accordingly
        if( (v == SC_LOGIC_Z) and (m_reg.pinpull & bit) ) {
          v = ( m_reg.pulldir & bit ) ? SC_LOGIC_1 : SC_LOGIC_0;
        }
        m_reg.datainp |= ( v == SC_LOGIC_1 ) ? bit : 0;
      }
      reg.datainp = m_reg.datainp;
      break;
    }
    case /*read*/ GPIO_DATAOUT_REG: // x5 =
    {
      reg.dataout = m_reg.dataout;
      break;
    }
    case /*read*/ GPIO_DATASET_REG: // x6 |=
    {
      reg.dataset = m_reg.dataset;
      break;
    }
    case /*read*/ GPIO_DATACLR_REG: // x7 &= ~
    {
      reg.dataclr = m_reg.dataclr;
      break;
    }
    case /*read*/ GPIO_DATAINV_REG: // x8 ^=
    {
      reg.datainv = reg.datainv;
      break;
    }
    case /*read*/ GPIO_DATACHG_REG: // x9 input change detected / write 1 to clear
    {
      reg.datachg = m_reg.datachg;
      break;
    }
    case /*read*/ GPIO_DATAENA_REG: // xA write 1's to clear changed bits
    {
      reg.dataena = m_reg.dataena;
      break;
    }
    case /*read*/ GPIO_PINRISE_REG: // xB 1's indicate rising detection
    {
      reg.pinrise = m_reg.pinrise;
      break;
    }
    case /*read*/ GPIO_PINFALL_REG: // xC 1's indicate falling detection
    {
      reg.pinfall = m_reg.pinfall;
      break;
    }
    default: // ignore
    {
      break;
    }
  }//endswitch
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos Inc. All rights reserved.
//END gpio.cpp @(#)$Id$
