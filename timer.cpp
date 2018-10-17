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
#include "timer.hpp"
#include "timer_regs.hpp"
#include "report.hpp"
#include "util.hpp"
#include "config_extn.hpp"
#include <algorithm>
namespace {
  const char* MSGID{"/Doulos/Example/TLM-Timer"};
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
Timer_module::Timer_module // Constructor
( sc_module_name instance_name
, Depth_t        target_depth
, Addr_t         target_start
, size_t         timer_quantity // Number of timers
, uint32_t       addr_clocks
, uint32_t       read_clocks
, uint32_t       write_clocks
)
: m_target_depth            { target_depth    }
//m_target_start not needed
, m_addr_clocks             { addr_clocks     }
, m_read_clocks             { read_clocks     }
, m_write_clocks            { write_clocks    }
, m_targ_peq                { this, &Timer_module::targ_peq_cb }
{
  SC_HAS_PROCESS( Timer_module );
  SC_THREAD( timer_thread );
  m_reg_vec.size( timer_qty * TIMER_SIZE );
  m_timer_vec.size( timer_qty );
  SC_HAS_PROCESS( Timer_module );
  SC_METHOD( execute_transaction_process );
    sensitive << m_target_done_event;
    dont_initialize();
  targ_socket.register_b_transport        ( this, &Timer_module::b_transport );
  targ_socket.register_nb_transport_fw    ( this, &Timer_module::nb_transport_fw );
  targ_socket.register_transport_dbg      ( this, &Timer_module::transport_dbg );
  m_config.set( "name",         string(name())  );
  m_config.set( "kind",         string(kind())  );
  m_config.set( "object_ptr",   uintptr_t(this) );
  m_config.set( "target_start", target_start    );
  m_config.set( "target_depth", target_depth    );
  m_config.set( "addr_clocks",  addr_clocks     );
  m_config.set( "read_clocks",  read_clocks     );
  m_config.set( "write_clocks", write_clocks    );
  m_config.set( "coding_style", Style::AT       );
  INFO( ALWAYS, "Constructed " << name() << " with config:\n" << m_config ); }

//------------------------------------------------------------------------------
// Destructor
Timer_module::~Timer_module( void )
{
  INFO( ALWAYS, "Destroyed " << name() );
}

//------------------------------------------------------------------------------
void Timer_module::timer_thread( void )
{
  // Setup events to monitor
  sc_event_or_list events;
  for( auto& v : m_timer_vec ) {
    events |= v.timeout_event() | v.pulse_width_event();
  }
  for(;;) {
    wait( events );
    for( auto& v : m_timer_vec ) {
      if( sc_time_stamp() == v.get_load_time() ) {
      }
      if( sc_time_stamp() == v.get_pulse_time() ) {
      }
    }
  }//endforever
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Timer_module::b_transport
( Timer_module::tlm_payload_t& trans
, sc_time& delay
)
{

  Depth_t len = trans.get_data_length();

  if( not payload_is_ok( trans, len, Style::LT ) ) {
    return;
  }
  transport( trans, delay, len );
}

//------------------------------------------------------------------------------
Depth_t
Timer_module::transport_dbg
( Timer_module::tlm_payload_t& trans
)
{
  INFO( DEBUG, "Executing " << name() << "." << __func__ << "::transport_dbg" );
  bool config_only{config(trans)};
  if( config_only ) {
    INFO( DEBUG, "config_only" );
    trans.set_response_status( TLM_OK_RESPONSE );
    return 0;
  }
  // Allow for length beyond end by truncating
  Addr_t  adr = trans.get_address();
  Depth_t len = trans.get_data_length();
  if( ( adr + len - 1 ) > m_target_depth ) {
    len -= ( adr + len - 1 ) - m_target_depth;
  }

  if( not payload_is_ok( trans, len, Style::LT ) ) {
    return 0;
  }
  sc_time delay( SC_ZERO_TIME );
  return transport( trans, delay, len );
}

//------------------------------------------------------------------------------
// Return true if configuration is all that is needed
bool Timer_module::config ( tlm_payload_t& trans)
{
  Config_extn* extn{trans.get_extension<Config_extn>()};
  if( extn != nullptr ) {
    INFO( DEBUG, "Configuring " << name() );
    if (extn->config.empty()) {
      NOINFO( DEBUG, "Sending config:\n" << m_config );
      extn->config = m_config;
    } else {                                   
      m_config.update( extn->config );
      // Update local copies
      extn->config.get( "target_depth", m_target_depth );
      extn->config.get( "addr_clocks" , m_addr_clocks  );
      extn->config.get( "read_clocks" , m_read_clocks  );
      extn->config.get( "write_clocks", m_write_clocks );
      INFO( DEBUG, "Updated config " << m_config );
    }
  }
  trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
  return trans.get_command() == TLM_IGNORE_COMMAND;
}

//------------------------------------------------------------------------------
void Timer_module::execute_transaction( Timer_module::tlm_payload_t& trans )
{
  Depth_t len = trans.get_data_length();
  if( not payload_is_ok( trans, len, Style::AT ) ) {
    return;
  }
  sc_time delay( SC_ZERO_TIME );
  transport( trans, delay, len );
}

//------------------------------------------------------------------------------
tlm_sync_enum
Timer_module::nb_transport_fw
( Timer_module::tlm_payload_t& trans
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
Timer_module::targ_peq_cb
( Timer_module::tlm_payload_t& trans
, const Timer_module::tlm_phase_t& phase
)
{
  sc_time delay;

  switch ( phase ) {
  case BEGIN_REQ:

    // Increment the transaction reference count
    trans.acquire();

    if( m_transaction_in_progress != nullptr )
      send_end_req( trans );
    else
      // Put back-pressure on initiator by deferring END_REQ until pipeline is clear
      m_end_req_pending = &trans;

    break;

  case END_RESP:
    // On receiving END_RESP, the target can release the transaction
    // and allow other pending transactions to proceed

    if( not m_response_in_progress )
      REPORT( FATAL, "Illegal transaction phase END_RESP received by target" );

    // Flag must only be cleared when END_RESP is sent
    m_transaction_in_progress = nullptr;

    // Target itself is now clear to issue the next BEGIN_RESP
    m_response_in_progress = false;
    if( m_next_response_pending != nullptr )
    {
      send_response( *m_next_response_pending );
      m_next_response_pending = nullptr;
    }

    // ... and to unblock the initiator by issuing END_REQ
    if( m_end_req_pending != nullptr )
    {
      send_end_req( *m_end_req_pending );
      m_end_req_pending = nullptr;
    }

    break;

  case END_REQ:
  case BEGIN_RESP:
    REPORT( FATAL, "Illegal transaction phase received by target" );
    break;
  }
}

//------------------------------------------------------------------------------
void 
Timer_module::send_end_req( Timer_module::tlm_payload_t& trans )
{
  Timer_module::tlm_phase_t bw_phase;
  sc_time delay;

  // Queue the acceptance and the response with the appropriate latency
  bw_phase = END_REQ;
  delay = rand_ps(5); // Accept delay

  tlm_sync_enum status = targ_socket->nb_transport_bw( trans, bw_phase, delay );
  // Ignore return value; initiator cannot terminate transaction at this point

  // Queue internal event to mark beginning of response
  delay = delay + rand_ps(5); // Latency
  m_target_done_event.notify( delay );

  assert( m_transaction_in_progress == nullptr );
  m_transaction_in_progress = &trans;
}

//------------------------------------------------------------------------------
void
Timer_module::send_response( Timer_module::tlm_payload_t& trans )
{
  tlm_sync_enum status;
  Timer_module::tlm_phase_t bw_phase;
  sc_time delay;

  m_response_in_progress = true;
  bw_phase = BEGIN_RESP;
  delay = SC_ZERO_TIME;
  status = targ_socket->nb_transport_bw( trans, bw_phase, delay );

  if( status == TLM_UPDATED )
  {
    // The timing annotation must be honored
    m_targ_peq.notify( trans, bw_phase, delay );
  }
  else if( status == TLM_COMPLETED )
  {
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
  if( m_response_in_progress ) {
    // Target allows only two transactions in-flight
    if( m_next_response_pending != nullptr )
      REPORT( FATAL, "Attempt to have two pending responses in target" );
    m_next_response_pending = m_transaction_in_progress;
  }
  else {
    send_response( *m_transaction_in_progress );
  }
}

//------------------------------------------------------------------------------
bool Timer_module::payload_is_ok( Timer_module::tlm_payload_t& trans, Depth_t len, Style coding_style )
{
  tlm_command cmd = trans.get_command();
  Addr_t      adr = trans.get_address();
  uint8_t*    ptr = trans.get_data_ptr();
  uint8_t*    byt = trans.get_byte_enable_ptr();
  Depth_t     wid = trans.get_streaming_width();

  if( ( adr+len ) >= m_target_depth or (addr & 3) != 0) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << adr );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    return false;
  } 
  else if( byt != 0 ) { 
    if( g_error_at_target ) {
      REPORT( ERROR, "Attempt to unsupported use byte enables " << name() << " with address " << adr );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_BYTE_ENABLE_ERROR_RESPONSE );
    }
    return false;
  }
  else if(  (len & 3) != 0 or coding_style == Style::AT and m_max_burst > 0 and len > m_max_burst ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Attempt to burst " << len << " bytes to " << name() << " with address " << adr << " when max burst size is " << m_max_burst );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_BURST_ERROR_RESPONSE );
    }
    return false;
  }
  else if( wid < len ) { // No streaming
    if( g_error_at_target ) {
      REPORT( ERROR, "Attempt to stream to " << name() << " with address " << adr );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_GENERIC_ERROR_RESPONSE );
    }
    return false;
  }
  else if( cmd == TLM_WRITE_COMMAND ) { // No extended commands
    if( g_error_at_target ) {
      REPORT( ERROR, "Attempt to write read-only device " << name() << " with address " << adr );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_COMMAND_ERROR_RESPONSE );
    }
    return false;
  } else {
    return true;
  }
}

//------------------------------------------------------------------------------
Depth_t
Timer_module::transport
( Timer_module::tlm_payload_t& trans
  , sc_time& delay
  , Depth_t  len
)
{
  Addr_t     adr = trans.get_address();
  uint8_t*   ptr = trans.get_data_ptr();
  Depth_t    sbw = targ_socket.get_bus_width() / 8;
  sc_assert( adr + len < m_target_depth );
  uint8_t*   reg = m_reg_vec.data();
  delay += clk.clocks( m_addr_clocks );

  if ( trans.is_write() ) {
    INFO( DEBUG, "Writing " << HEX << adr << "..." << ( adr + len - 1 ) );
    memcpy( reg + adr, ptr, len );
    delay += clk.clocks( m_write_clocks ) * ( ( len + sbw - 1 ) / sbw );
    write_actions( adr, ptr, len );
  }
  else if ( trans.is_read() ) {
    read_actions( adr, ptr, len );
    INFO( DEBUG, "Reading " << HEX << adr << "..." << ( adr + len - 1 ) );
    memcpy( ptr, reg + adr, len );
    delay += clk.clocks( m_read_clocks ) * ( ( len + sbw - 1 ) / sbw );
  }
  else {
    len = 0;
  }

  trans.set_response_status( TLM_OK_RESPONSE );
  return len;
}

//------------------------------------------------------------------------------
void write_actions( Addr_t address, uint8_t* data_ptr, Depth_t len )
{
  int index           { int(address/TIMER_SIZE) }; 
  Addr_t base_address { TIMER_SIZE*index };
  Addr_t reg_address  { address - base_address };
  if( base_address == TIMER_GLOBAL_REG ) {
  } else {
    Timer_reg& timer_reg { reinterpret_cast<Timer_ address - base_address>(data_ptr[ base_address ]) };
    if( len == sizeof(uint32_t) ) {
      switch ( reg_address ) {
        case TIMER_STATUS_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_CTRLSET_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_CTRLCLR_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_LOAD_LO_REG:
          timer_reg.load_hi = 0;
          sc_time load_time = clock_period*(timer_reg.load_lo);
          m_timer_vec[index].set_load_time( load_time );
          break;
        case TIMER_LOAD_HI_REG:
          sc_time load_time = clock_period*((uint64_t(timer_reg.load_hi)<<32) + timer_reg.load_lo);
          m_timer_vec[index].set_load_time( load_time );
          break;
        case TIMER_CURR_LO_REG:
          timer_reg.curr_hi = 0;
          sc_time curr_time = clock_period*(timer_reg.curr_lo);
          m_timer_vec[index].set_timeout_time( curr_time );
          break;
        case TIMER_CURR_HI_REG:
          sc_time curr_time = clock_period*((uint64_t(timer_reg.curr_hi)<<32) + timer_reg.curr_lo);
          m_timer_vec[index].set_timeout_time( curr_time );
          break;
        case TIMER_PULSE_REG:
          sc_time pulse_time = clock_period*timer_reg.pulse;
          break;
      }
    } else {
      sc_assert( len == 2*sizeof(uint32_t) );
      switch ( reg_address ) {
        case TIMER_LOAD_LO_REG:
          sc_time load_time = clock_period*((uint64_t(timer_reg.load_hi)<<32) + timer_reg.load_lo);
          m_timer_vec[index].set_load_time( load_time );
          break;
        case TIMER_CURR_LO_REG:
          sc_time curr_time = clock_period*((uint64_t(timer_reg.curr_hi)<<32) + timer_reg.curr_lo);
          m_timer_vec[index].set_timeout_time( curr_time );
          break;
        default:
          break;
      }
    }
  }//endif
}

//------------------------------------------------------------------------------
void read_actions ( Addr_t address, uint8_t* data_ptr, Depth_t len )
{
  Addr_t base_address { int(address/TIMER_SIZE) };
  Addr_t reg_address  { address - base_address };
  if( base_address == TIMER_GLOBAL_REG ) {
  } else {
    Timer_reg& timer_reg { reinterpret_cast<Timer_ address - base_address>(data_ptr[ base_address ]) };
    sc_time load_time = m_timer_vec[index].get_timeout_time();
    uint64_t load_clocks = load_time/clock_period;
    sc_time curr_time = m_timer_vec[index].get_load_time();
    uint64_t curr_clocks = (m_timer_vec[index].curr_time() - m_timer_vec[index].get_start_time())/clock_period;
    NOT_YET_IMPLEMENTED();
    if( len = sizeof(uint32_t) ) {
      switch ( reg_address ) {
        case TIMER_STATUS_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_CTRLSET_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_CTRLCLR_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_LOAD_LO_REG:
          break;
        case TIMER_LOAD_HI_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_CURR_LO_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_CURR_HI_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_PULSE_REG:
          NOT_YET_IMPLEMENTED();
          break;
      }
    } else {
      switch ( reg_address ) {
        case TIMER_LOAD_LO_REG:
          NOT_YET_IMPLEMENTED();
          break;
        case TIMER_CURR_LO_REG:
          NOT_YET_IMPLEMENTED();
          break;
        default:
          break;
      }
    }
  }//endif
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
