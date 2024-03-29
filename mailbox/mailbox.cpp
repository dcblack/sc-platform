#include "memory/memory.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//  #     # ##### #     #  ####  #####  #     #        #     # ####   #           
//  ##   ## #     ##   ## #    # #    #  #   #         ##   ## #   #  #           
//  # # # # #     # # # # #    # #    #   # #          # # # # #    # #           
//  #  #  # ##### #  #  # #    # #####     #           #  #  # #    # #           
//  #     # #     #     # #    # #  #      #           #     # #    # #           
//  #     # #     #     # #    # #   #     #           #     # #   #  #           
//  #     # ##### #     #  ####  #    #    #    ###### #     # ####   #####       
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
#include "config/config_extn.hpp"
#include <algorithm>
#include <random>
namespace {
  char const * const MSGID{ "/Doulos/Example/TLM-Memory" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
Memory_module::Memory_module // Constructor
( sc_module_name instance_name
, Depth_t        target_size
, Addr_t         target_base
, Access         access    
, size_t         max_burst
, size_t         alignment
, Feature        dmi_allowed
, Feature        byte_enables
, uint32_t       addr_clocks
, uint32_t       read_clocks
, uint32_t       write_clocks
)
: m_target_size            { target_depth    }
//m_target_base not needed
, m_dmi_allowed             { dmi_allowed     }
, m_access                  { access          }
, m_byte_enables            { byte_enables    }
, m_alignment               { alignment       }
, m_max_burst               { max_burst       }
, m_addr_clocks             { addr_clocks     }
, m_read_clocks             { read_clocks     }
, m_write_clocks            { write_clocks    }
, m_targ_peq                { "targ_peq", this, &Memory_module::targ_peq_cb }
{
  SC_HAS_PROCESS( Memory_module );
  SC_METHOD( execute_transaction_process );
    sensitive << m_target_done_event;
    dont_initialize();
  targ_socket.register_b_transport        ( this, &Memory_module::b_transport );
  targ_socket.register_nb_transport_fw    ( this, &Memory_module::nb_transport_fw );
  targ_socket.register_get_direct_mem_ptr ( this, &Memory_module::get_direct_mem_ptr );
  targ_socket.register_transport_dbg      ( this, &Memory_module::transport_dbg );
  m_configuration.set( "name",         string(name())  );
  m_configuration.set( "kind",         string(kind())  );
  m_configuration.set( "object_ptr",   uintptr_t(this) );
  m_configuration.set( "target_base", target_start    );
  m_configuration.set( "target_size", target_depth    );
  m_configuration.set( "dmi_allowed",  dmi_allowed     );
  m_configuration.set( "access",       access          );
  m_configuration.set( "byte_enables", byte_enables    );
  m_configuration.set( "alignment",    alignment       );
  m_configuration.set( "max_burst",    max_burst       );
  m_configuration.set( "addr_clocks",  addr_clocks     );
  m_configuration.set( "read_clocks",  read_clocks     );
  m_configuration.set( "write_clocks", write_clocks    );
  m_configuration.set( "coding_style", Style::AT       );
  INFO( ALWAYS, "Constructed " << name() << " with configuration:\n" << m_configuration );
}

//------------------------------------------------------------------------------
// Destructor
Memory_module::~Memory_module( void )
{
  INFO( ALWAYS, "Destroyed " << name() );
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Memory_module::b_transport
( Memory_module::tlm_payload_t& trans
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
Memory_module::transport_dbg
( Memory_module::tlm_payload_t& trans
)
{
  INFO( DEBUG, "Executing " << name() << "." << __func__ << "::transport_dbg" );
  bool config_only{configure(trans)};
  // shrink as needed
  if( m_target_size < m_mem_vec.size() ) {
    resize( m_target_size );
  }
  if( config_only ) {
    INFO( DEBUG, "config_only" );
    trans.set_response_status( TLM_OK_RESPONSE );
    return 0;
  }
  // Allow for length beyond end by truncating
  Addr_t  adr = trans.get_address();
  Depth_t len = trans.get_data_length();
  if( ( adr + len - 1 ) > m_target_size ) {
    len -= ( adr + len - 1 ) - m_target_size;
  }

  if( not payload_is_ok( trans, len, Style::LT ) ) {
    return 0;
  }
  sc_time delay( SC_ZERO_TIME );
  return transport( trans, delay, len );
}

//------------------------------------------------------------------------------
// Return true if configuration is all that is needed
bool Memory_module::configure( tlm_payload_t& trans )
{
  Config_extn* extn{trans.get_extension<Config_extn>()};
  if( extn != nullptr ) {
    INFO( DEBUG, "Configuring " << name() );
    if (extn->configuration.empty()) {
      NOINFO( DEBUG, "Sending configuration:\n" << m_configuration );
      extn->configuration = m_configuration;
    } else {                                   
      m_configuration.update( extn->configuration );
      // Update local copies
      extn->configuration.get( "target_size", m_target_depth   );
      extn->configuration.get( "dmi_allowed ", m_dmi_allowed   );
      extn->configuration.get( "access"      , m_access        );
      extn->configuration.get( "byte_enables", m_byte_enables  );
      extn->configuration.get( "alignment"   , m_alignment     );
      extn->configuration.get( "max_burst"   , m_max_burst     );
      extn->configuration.get( "addr_clocks" , m_addr_clocks   );
      extn->configuration.get( "read_clocks" , m_read_clocks   );
      extn->configuration.get( "write_clocks", m_write_clocks  );
      INFO( DEBUG, "Updated configuration " << m_configuration );
    }
  }
  trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
  return trans.get_command() == TLM_IGNORE_COMMAND;
}

//------------------------------------------------------------------------------
void Memory_module::resize( int depth, int pattern )
{
  sc_assert( depth > 0 and depth <= m_target_size );
  m_mem_vec.resize( depth, pattern );
  m_used_vec.resize( depth, false );
}

//------------------------------------------------------------------------------
void Memory_module::execute_transaction( Memory_module::tlm_payload_t& trans )
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
Memory_module::nb_transport_fw
( Memory_module::tlm_payload_t& trans
, tlm_phase& phase
, sc_time& delay
)
{

  // Queue the transaction until the annotated time has elapsed
  m_targ_peq.notify( trans, phase, delay );
  return TLM_ACCEPTED;
}

//------------------------------------------------------------------------------
bool
Memory_module::get_direct_mem_ptr
( tlm_generic_payload& trans
, tlm_dmi& dmi_data
)
{
  dmi_data.init();
  if( m_dmi_allowed == DMI::none ) {
    return false;
  }
  m_dmi_granted = true;
  dmi_data.set_dmi_ptr( m_mem_vec.data() );
  dmi_data.set_start_address( 0 );
  dmi_data.set_end_address( m_target_size - 1 );
  dmi_data.set_read_latency( clk.period( m_read_clocks ) );
  dmi_data.set_write_latency( clk.period( m_write_clocks ) );
  if( m_access == Access::RO ) {
    dmi_data.allow_read();
  } else {
    dmi_data.allow_read_write();
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Helpers

//------------------------------------------------------------------------------
void
Memory_module::targ_peq_cb
( Memory_module::tlm_payload_t& trans
, const Memory_module::tlm_phase_t& phase
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
Memory_module::send_end_req( Memory_module::tlm_payload_t& trans )
{
  Memory_module::tlm_phase_t bw_phase;
  sc_time delay;
  // Random number generation
  static std::default_random_engine generator;
  std::normal_distribution<double> distribution{ 5.0, 0.0 };

  // Queue the acceptance and the response with the appropriate latency
  bw_phase = END_REQ;
  delay = sc_time( distribution( generator ), SC_PS ); // Accept delay

  tlm_sync_enum status = targ_socket->nb_transport_bw( trans, bw_phase, delay );
  // Ignore return value; initiator cannot terminate transaction at this point

  // Queue internal event to mark beginning of response
  delay = delay + sc_time( distribution( generator ), SC_PS ); // Latency
  m_target_done_event.notify( delay );

  assert( m_transaction_in_progress == nullptr );
  m_transaction_in_progress = &trans;
}

//------------------------------------------------------------------------------
void
Memory_module::send_response( Memory_module::tlm_payload_t& trans )
{
  tlm_sync_enum status;
  Memory_module::tlm_phase_t bw_phase;
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
Memory_module::execute_transaction_process( void )
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
bool Memory_module::payload_is_ok( Memory_module::tlm_payload_t& trans, Depth_t len, Style coding_style )
{
  tlm_command cmd = trans.get_command();
  Addr_t      adr = trans.get_address();
  uint8_t*    ptr = trans.get_data_ptr();
  uint8_t*    byt = trans.get_byte_enable_ptr();
  Depth_t     wid = trans.get_streaming_width();

  if( ( adr+len ) >= m_target_size ) {
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
  else if( coding_style == Style::AT and m_max_burst > 0 and len > m_max_burst ) {
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
  else if( m_access == Access::RO and cmd == TLM_WRITE_COMMAND ) {
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
Memory_module::transport
( Memory_module::tlm_payload_t& trans
, sc_time& delay
, Depth_t  len
)
{
  Addr_t     adr = trans.get_address();
  uint8_t*   ptr = trans.get_data_ptr();
  Depth_t    sbw = targ_socket.get_bus_width()/8;
  sc_assert( adr+len < m_target_size );
  if( m_mem_vec.size() < (adr+len) ) {
    // Expand to nearest 1K
    Depth_t new_size = (adr+len) + 1*KB;
    // Limited by configured size
    new_size = std::min( new_size, m_target_size );
    resize( new_size );
  }
  uint8_t*   mem = m_mem_vec.data();
  delay += clk.period( m_addr_clocks );
  INFO( DEBUG+1, "Transport to address " << STREAM_HEX << adr << " in " << name() );
  if( trans.is_read() ) {
    // TODO: Add byte enable support
    memcpy( ptr, mem+adr, len );
    delay += clk.period( m_read_clocks ) * ( ( len+sbw-1 )/sbw );
  }
  else if( trans.is_write() ) {
    // TODO: Add byte enable support
    memcpy( mem+adr, ptr, len );
    delay += clk.period( m_write_clocks ) * ( ( len+sbw-1 )/sbw );
    if( m_used_vec.size() ) {
      for( auto a=adr; a<( adr+len ); ++a ) {
        m_used_vec[a] = true;
      }
    }
  } else {
    len = 0;
  }
  trans.set_response_status( TLM_OK_RESPONSE );
  return len;
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
