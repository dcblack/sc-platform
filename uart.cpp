#include "usart.hpp"
#include "report.hpp"
#include "config_extn.hpp"
#include <algorithm>
#include <random>
namespace {
  const char* const MSGID{"/Doulos/{:PROJECT:}/Usart"};
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
Usart_module::Usart_module // Constructor
( sc_module_name instance_name
, uint32_t       addr_clocks
, uint32_t       read_clocks
, uint32_t       write_clocks
)
: m_addr_clocks             { addr_clocks     }
, m_read_clocks             { read_clocks     }
, m_write_clocks            { write_clocks    }
, m_targ_peq                { this, &Usart_module::targ_peq_cb }
{
  SC_HAS_PROCESS( Usart_module );
  SC_THREAD( usart_thread );
  SC_METHOD( execute_transaction_process );
    sensitive << m_target_done_event;
    dont_initialize();
  targ_socket.register_b_transport        ( this, &Usart_module::b_transport );
  targ_socket.register_nb_transport_fw    ( this, &Usart_module::nb_transport_fw );
  targ_socket.register_transport_dbg      ( this, &Usart_module::transport_dbg );
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
Usart_module::~Usart_module( void )
{
  INFO( ALWAYS, "Destroyed " << name() );
}

//------------------------------------------------------------------------------
void Usart_module::end_of_elaboration( void )
{
  //{:TO BE SUPPLIED -OR- DELETE ENTIRELY:}
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Usart_module::b_transport
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
Usart_module::transport_dbg
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
bool Usart_module::configure( tlm_payload_t& trans )
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
void Usart_module::execute_transaction( tlm_payload_t& trans )
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
Usart_module::nb_transport_fw
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
Usart_module::targ_peq_cb
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
Usart_module::send_end_req( tlm_payload_t& trans )
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
Usart_module::send_response( tlm_payload_t& trans )
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
Usart_module::execute_transaction_process( void )
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
bool Usart_module::payload_is_ok
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
Usart_module::transport ( tlm_payload_t& trans, sc_time& delay)
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
// Usart main actions
//------------------------------------------------------------------------------
void Usart_module::usart_thread( void )
{
  for(;;) {
    {:REPLACE WITH FUNCTIONALITY AS NEEDED -OR- DELETE ENTIRELY:}
  }
}//end Usart_module::usart_thread()

//------------------------------------------------------------------------------
void Usart_module::write_actions( tlm_payload_t& trans, const sc_time& delay )
{
  // Make it easier to access data
  Usart_regs_t& reg{ *reinterpret_cast<Usart_regs_t*>( m_register_vec.data() ) };
  Addr_t address = trans.get_address();
  Addr_t reg_address = address & ~3ull; // Clear lower 4-bits (assumes word access)

  // Word transfer
  switch ( reg_address ) {
    case /*write*/ {:ADDRESS:}:
    {
      {:ACTION:};
      break;
    }
  }
}

//------------------------------------------------------------------------------
void Usart_module::read_actions( tlm_payload_t& trans, const sc_time& delay )
{
  // Make it easier to access data
  Usart_regs_t& reg{ *reinterpret_cast<Usart_regs_t*>( m_register_vec.data() ) };
  Addr_t address = trans.get_address();
  Addr_t reg_address = address & ~3ull; // Clear lower 4-bits (assumes word access)

  switch ( reg_address ) {
    case /*write*/ {:ADDRESS:}:
    {
      {:ACTION:};
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END usart.cpp @(#)$Id$
