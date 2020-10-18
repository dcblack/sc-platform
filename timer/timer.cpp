#include "timer/timer.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//  ####### ### #     # ##### #####         #     # ####   #
//     #     #  ##   ## #     #    #        ##   ## #   #  #
//     #     #  # # # # #     #    #        # # # # #    # #
//     #     #  #  #  # ##### #####         #  #  # #    # #
//     #     #  #     # #     #  #          #     # #    # #
//     #     #  #     # #     #   #         #     # #   #  #
//     #    ### #     # ##### #    # ###### #     # ####   #####
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
#include "common/log2.hpp"
#include "config/config_extn.hpp"
#include <algorithm>
namespace {
  char const * const MSGID{ "/Doulos/Example/TLM-Timer" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
Timer_module::Timer_module // Constructor
( sc_module_name instance_name
  , size_t         timer_quantity // Number of timers
  , uint32_t       addr_clocks
  , uint32_t       read_clocks
  , uint32_t       write_clocks
)
  : m_target_size             { Depth_t( timer_quantity * TIMER_REGS_SIZE + sizeof( uint32_t ) ) }
  , m_timer_quantity          { timer_quantity  }
  , m_addr_clocks             { addr_clocks     }
  , m_read_clocks             { read_clocks     }
  , m_write_clocks            { write_clocks    }
  , m_timer_vec               { "timer"         }
  , m_targ_peq                { "target_peq", this, &Timer_module::targ_peq_cb }
{
  SC_HAS_PROCESS( Timer_module );
  SC_THREAD( timer_thread );
  m_register_vec.resize( timer_quantity * TIMER_REGS_SIZE );
  m_timer_vec.init( timer_quantity );
  SC_HAS_PROCESS( Timer_module );
  SC_METHOD( execute_transaction_process );
  sensitive << m_target_done_event;
  dont_initialize();
  targ_socket.register_b_transport     ( this, &Timer_module::b_transport );
  targ_socket.register_nb_transport_fw ( this, &Timer_module::nb_transport_fw );
  targ_socket.register_transport_dbg   ( this, &Timer_module::transport_dbg );
  m_configuration.set( "name",         string( name() )  );
  m_configuration.set( "kind",         string( kind() )  );
  m_configuration.set( "object_ptr",   uintptr_t( this ) );
  m_configuration.set( "target_size",  m_target_size     );
  m_configuration.set( "addr_clocks",  addr_clocks       );
  m_configuration.set( "read_clocks",  read_clocks       );
  m_configuration.set( "write_clocks", write_clocks      );
  m_configuration.set( "coding_style", Style::AT         );
  INFO( ALWAYS, "Constructed " << name() << " with configuration:\n" << m_configuration );
}

//------------------------------------------------------------------------------
// Destructor
Timer_module::~Timer_module( void )
{
  INFO( ALWAYS, "Destroyed " << name() );
}

//------------------------------------------------------------------------------
void Timer_module::end_of_elaboration( void )
{
  // What use is a timer module if not connected to something?
  int connections = intrq_port.size() + pulse_port.size();
  MESSAGE( "Timer " << name() << " has " 
    << (intrq_port.size() ? "connected" : "no") << " interrupt ports"
    << " and " << pulse_port.size() << " pulse ports connected."
  );
  if( connections == 0 ) {
    REPORT( WARNING, " Did you forget something?" );
  } else {
    MEND( ALWAYS );
  }
}

//------------------------------------------------------------------------------
void Timer_module::timer_thread( void )
{
  sc_event_or_list trigger_events;

  for(;;) {

    // Setup events to monitor
    for ( auto& v : m_timer_vec ) {
      trigger_events |= v.trigger_event();
    }
    wait( trigger_events );

    if( intrq_port.size() == 1 ) {
      // Generate interrupt if any enabled timers has triggered
      for( int iIrq=0; iIrq != intrq_port.size(); ++iIrq ) {
        if( m_timer_vec[ iIrq ].get_triggered() and irq_enabled( iIrq ) ) {
          timer_reg_vec( iIrq ).status |= TIMER_IRQ( iIrq );
          intrq_port->notify( name() );
          break;
        }
      }
    }

    // Generate pulse for connected ports
    for( int iPulse=0; iPulse!=pulse_port.size(); ++iPulse ) {
      if( m_timer_vec[iPulse].test_and_clear_triggered() ) {
        sc_spawn( [&]() 
          {
            pulse_port[iPulse]->write(true);
            wait( m_timer_vec[iPulse].get_pulse_delay() );
            pulse_port[iPulse]->write(false);
          }
        );
      }
    }//end pulse

  }//endforever
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Timer_module::b_transport
( tlm_payload_t& trans
, sc_time&       delay
)
{

  Depth_t len = trans.get_data_length();

  if ( not payload_is_ok( trans, len, Style::LT ) ) {
    return;
  }

  transport( trans, delay, len );
}

//------------------------------------------------------------------------------
Depth_t
Timer_module::transport_dbg
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
bool Timer_module::configure( tlm_payload_t& trans )
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
      extn->configuration.get( "target_size",  m_target_size   );
      extn->configuration.get( "addr_clocks",  m_addr_clocks   );
      extn->configuration.get( "read_clocks",  m_read_clocks   );
      extn->configuration.get( "write_clocks", m_write_clocks  );
      INFO( DEBUG, "Updated configuration " << m_configuration );
    }
  }

  trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
  return trans.get_command() == TLM_IGNORE_COMMAND;
}

//------------------------------------------------------------------------------
void Timer_module::execute_transaction( tlm_payload_t& trans )
{
  Depth_t len = trans.get_data_length();

  if ( not payload_is_ok( trans, len, Style::AT ) ) {
    return;
  }

  sc_time delay( SC_ZERO_TIME );
  transport( trans, delay, len );
}

//------------------------------------------------------------------------------
tlm_sync_enum
Timer_module::nb_transport_fw
( tlm_payload_t& trans
, tlm_phase&     phase
, sc_time&       delay
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
Timer_module::targ_peq_cb
( tlm_payload_t&     trans
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
Timer_module::send_end_req( tlm_payload_t& trans )
{
  tlm_phase_t bw_phase;
  sc_time     delay;

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
Timer_module::send_response( tlm_payload_t& trans )
{
  tlm_sync_enum status;
  tlm_phase_t   bw_phase;
  sc_time       delay;

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
Timer_module::execute_transaction_process( void )
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
bool Timer_module::payload_is_ok
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
Timer_module::transport
( tlm_payload_t& trans
, sc_time&       delay
, Depth_t        len
)
{
  Addr_t     adr = trans.get_address();
  uint8_t*   ptr = trans.get_data_ptr();
  Depth_t    sbw = targ_socket.get_bus_width() / 8;
  sc_assert( adr + len < m_target_size );
  uint8_t*   reg = m_register_vec.data();
  delay += clk.period( m_addr_clocks );

  INFO( DEBUG+1, "Transport to address " << STREAM_HEX << adr << " in " << name() );
  if ( trans.is_write() ) {
    memcpy( reg + adr, ptr, len );
    delay += clk.period( m_write_clocks ) * ( ( len + sbw - 1 ) / sbw );
    write_actions( adr, reg, len, delay );
  }
  else if ( trans.is_read() ) {
    read_actions( adr, reg, len, delay );
    memcpy( ptr, reg + adr, len );
    delay += clk.period( m_read_clocks ) * ( ( len + sbw - 1 ) / sbw );
  }
  else {
    len = 0;
  }

  trans.set_response_status( TLM_OK_RESPONSE );
  return len;
}//end Timer_module::transport

////////////////////////////////////////////////////////////////////////////////
// Timer Helpers

//------------------------------------------------------------------------------
// Map Timer_reg_t structure onto respective registers
Timer_reg_t& Timer_module::timer_reg_vec( int index )
{
  sc_assert( 0 <= index and index < m_timer_quantity );
  uint8_t* base_ptr      { m_register_vec.data() + index * TIMER_REGS_SIZE };
  Timer_reg_t* timer_reg { reinterpret_cast<Timer_reg_t*>( base_ptr ) };
  return *timer_reg;
}

//------------------------------------------------------------------------------
uint32_t Timer_module::get_timer_status
( unsigned int index )
{
  uint32_t status { ( uint32_t( m_timer_quantity ) << TIMER_QTY_LSB ) };
  sc_assert( status <= TIMER_QTY_MASK ); // Make sure it fits
  status |= timer_reg_vec( index ).status & TIMER_SCALE_MASK; // Preserve

  if ( m_timer_vec[index].get_reload() ) {
    status |= TIMER_RELOAD;
  }

  if ( m_timer_vec[index].get_continuous() ) {
    status |= TIMER_CONTINUOUS;
  }

  if ( m_timer_vec[index].is_running() ) {
    status |= TIMER_START;
  }

  status |= timer_reg_vec( index ).status & TIMER_INTERRUPT_MASK; // Preserve

  if ( not m_timer_vec[index].is_paused() ) {
    status |= TIMER_NORMAL;
  }

  status |= timer_reg_vec( index ).status & TIMER_IRQ_MASK; // Preserve
  return status;
}

//------------------------------------------------------------------------------
void Timer_module::set_timer_status
( unsigned int index, const sc_time& delay )
{
  // Read current
  volatile uint32_t& next_status { timer_reg_vec( index ).status };
  // Update new values
  next_status &= ~TIMER_QTY_MASK; // ignore
  // Handle interrupt enable
  // - nothing to do
  // Handle reload
  m_timer_vec[index].set_reload( ( next_status & TIMER_RELOAD_MASK ) == TIMER_RELOAD );
  // Handle continuous/one-shot
  m_timer_vec[index].set_continuous( ( next_status & TIMER_STATE_MASK ) == TIMER_CONTINUOUS );

  if ( m_timer_vec[index].is_running() ) {
    INFO( DEBUG, "Timer " << m_timer_vec[index].name() << " running." );
    // Handle start/stop
    if ( ( next_status & TIMER_STARTED_MASK ) == TIMER_STOP ) {
      m_timer_vec[index].stop( delay );
    }
  }
  else {
    INFO( DEBUG, "Timer " << m_timer_vec[index].name() << " not running." );
    // Handle pause/run
    if ( ( m_timer_vec[index].is_paused() )
         and ( ( next_status & TIMER_PAUSED_MASK ) != TIMER_PAUSED )
       ) {
      m_timer_vec[index].resume( delay );
    }

    if ( ( next_status & TIMER_STARTED_MASK ) == TIMER_START ) {
      m_timer_vec[index].start( delay );
    }

    // Handle pause/run
    if ( ( next_status & TIMER_PAUSED_MASK ) == TIMER_PAUSED ) {
      m_timer_vec[index].pause( delay );
    }
  }
}

//------------------------------------------------------------------------------
void Timer_module::write_actions
( Addr_t address, uint8_t* data_ptr, Depth_t len, const sc_time& delay )
{
  unsigned int index     { static_cast<unsigned int>( address >> bits(TIMER_REGS_SIZE) ) };
  Addr_t base_address    { index << bits(TIMER_REGS_SIZE) };
  Addr_t reg_address     { address - base_address };
  Timer_reg_t& timer_reg { timer_reg_vec(index) };
  Timer&     timer       { m_timer_vec[index] };

  if ( address == m_timer_quantity*TIMER_REGS_SIZE ) {
  }
  else {
    // Check the size of the transfer
    if ( len == sizeof( uint32_t ) ) {
      // Word transfer
      switch ( reg_address ) {
        case /*write*/ TIMER_STATUS_REG:
        {
          set_timer_status( index, delay );
          break;
        }
        case /*write*/ TIMER_CTRLSET_REG:
        {
          timer_reg.status  = get_timer_status( index );
          timer_reg.status |= (timer_reg.ctrlset & ~TIMER_QTY_MASK);
          set_timer_status( index, delay );
          break;
        }
        case /*write*/ TIMER_CTRLCLR_REG:
        {
          timer_reg.status = get_timer_status( index );
          timer_reg.status &= ~(timer_reg.ctrlset & ~TIMER_QTY_MASK);
          set_timer_status( index, delay );
          break;
        }
        case /*write*/ TIMER_LOAD_LO_REG:
        {
          timer_reg.load_hi = 0;
          uint32_t timer_scale = scale(timer_reg.status);
          sc_time load_delay = clk.period( timer_scale * timer_reg.load_lo );
          timer.set_load_delay( load_delay );
          break;
        }
        case /*write*/ TIMER_LOAD_HI_REG:
        {
          uint32_t timer_scale = scale(timer_reg.status);
          sc_time load_delay = clk.period( timer_scale * (( uint64_t( timer_reg.load_hi ) << 32 ) + timer_reg.load_lo ));
          timer.set_load_delay( load_delay );
          break;
        }
        case /*write*/ TIMER_CURR_LO_REG:
        {
          NOT_YET_IMPLEMENTED();
          timer_reg.curr_hi = 0;
          uint32_t timer_scale = scale(timer_reg.status);
          //uint64_t prev_count =  ( curr_time(delay) - get_start_time() )/clk.period(timer_scale);
          //uint64_t next_count = timer_reg.curr_lo;
          //timer.set_start_time( curr_time );
          break;
        }
        case /*write*/ TIMER_CURR_HI_REG:
        {
          NOT_YET_IMPLEMENTED();
          uint32_t timer_scale = scale(timer_reg.status);
          //uint64_t prev_count =  ( curr_time(delay) - get_start_time() )/clk.period(timer_scale);
          //uint64_t next_count = ( uint64_t( timer_reg.curr_hi ) << 32 ) + timer_reg.curr_lo;
          //timer.set_start_time( curr_time );
          break;
        }
        case /*write*/ TIMER_PULSE_REG:
        {
          // - nothing to do
          break;
        }
      }
    }
    else {
      // Double word transfer
      sc_assert( len == 2 * sizeof( uint32_t ) );

      switch ( reg_address ) {
        case /*write*/ TIMER_LOAD_LO_REG:
        {
          sc_time load_delay = clk.period( ( uint64_t( timer_reg.load_hi ) << 32 ) + timer_reg.load_lo );
          timer.set_load_delay( load_delay );
          break;
        }
        case /*write*/ TIMER_CURR_LO_REG:
        {
          sc_time curr_time = clk.period( ( uint64_t( timer_reg.curr_hi ) << 32 ) + timer_reg.curr_lo );
          timer.set_load_delay( curr_time );
          break;
        }
        default:
          break;
      }
    }
  }//endif
}

//------------------------------------------------------------------------------
void Timer_module::read_actions( Addr_t address, uint8_t* data_ptr, Depth_t len, const sc_time& delay )
{
  unsigned int index     { static_cast<unsigned int>( address >> bits(TIMER_REGS_SIZE) ) };
  Addr_t base_address    { index << bits(TIMER_REGS_SIZE) };
  Addr_t reg_address     { address - base_address };
  Timer_reg_t& timer_reg { timer_reg_vec(index) };
  Timer&     timer       { m_timer_vec[index] };

  if ( address == m_timer_quantity*TIMER_REGS_SIZE ) {
  }
  else {
    sc_time load_delay = timer.get_trigger_time();
    sc_time curr_time = timer.get_load_delay();

    if ( len == sizeof( uint32_t ) ) {
      switch ( reg_address ) {
        case /*read*/ TIMER_STATUS_REG:
        case /*read*/ TIMER_CTRLSET_REG:
        case /*read*/ TIMER_CTRLCLR_REG:
        {
          timer_reg.status = get_timer_status( index );
          break;
        }
        case /*read*/ TIMER_LOAD_LO_REG:
        {
          // - nothing to do
          break;
        }
        case /*read*/ TIMER_LOAD_HI_REG:
        {
          // - nothing to do
          break;
        }
        case /*read*/ TIMER_CURR_LO_REG:
        {
          sc_assert( timer.curr_time(delay) > timer.get_start_time() );
          uint32_t timer_scale = scale(timer_reg.status);
          uint64_t curr_count =  ( timer.curr_time(delay) - timer.get_start_time() )/clk.period(timer_scale);
          timer_reg.curr_lo = uint32_t(curr_count);
          break;
        }
        case /*read*/ TIMER_CURR_HI_REG:
        {
          sc_assert( timer.curr_time(delay) > timer.get_start_time() );
          uint32_t timer_scale = scale(timer_reg.status);
          uint64_t curr_count =  ( timer.curr_time(delay) - timer.get_start_time() )/clk.period(timer_scale);
          timer_reg.curr_hi = uint32_t(curr_count >> 32);
          break;
        }
        case /*read*/ TIMER_PULSE_REG:
        {
          // - nothing to do
          break;
        }
      }
    }
    else {
      switch ( reg_address ) {
        case /*read*/ TIMER_LOAD_LO_REG:
        {
          // - nothing to do
          break;
        }

        case /*read*/ TIMER_CURR_LO_REG:
        {
          sc_assert( timer.curr_time(delay) > timer.get_start_time() );
          uint32_t timer_scale = scale(timer_reg.status);
          uint64_t curr_count =  ( timer.curr_time(delay) - timer.get_start_time() )/clk.period(timer_scale);
          timer_reg.curr_hi = uint32_t(curr_count >> 32);
          timer_reg.curr_lo = uint32_t(curr_count);
          break;
        }

        default:
          break;
      }
    }
  }//endif
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
