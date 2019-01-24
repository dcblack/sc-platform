#include "interrupt/pic.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//  #####  ###  ####         #     # ####   #                                     
//  #    #  #  #    #        ##   ## #   #  #                                     
//  #    #  #  #             # # # # #    # #                                     
//  #####   #  #             #  #  # #    # #                                     
//  #       #  #             #     # #    # #                                     
//  #       #  #    #        #     # #   #  #                                     
//  #      ###  ####  ###### #     # ####   #####                                 
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
#include "common/log2.hpp"
#include "config/config_extn.hpp"
#include "cpu/cpuid_extn.hpp"
#include "bus/memory_map.hpp"
#include <algorithm>
namespace {
  const char* const MSGID{"/Doulos Inc/Example/Pic"};
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
Pic_module::Pic_module // Constructor
( sc_module_name instance_name
, uint32_t       addr_clocks
, uint32_t       read_clocks
, uint32_t       write_clocks
)
: m_addr_clocks             { addr_clocks     }
, m_read_clocks             { read_clocks     }
, m_write_clocks            { write_clocks    }
, m_targ_peq                { this, &Pic_module::targ_peq_cb }
{
  irq_recv_xport.bind( irq_channel.send_if );
  SC_HAS_PROCESS( Pic_module );
  SC_THREAD( pic_thread );
  SC_METHOD( execute_transaction_process );
    sensitive << m_target_done_event;
    dont_initialize();
  targ_socket.register_b_transport     ( this, &Pic_module::b_transport );
  targ_socket.register_nb_transport_fw ( this, &Pic_module::nb_transport_fw );
  targ_socket.register_transport_dbg   ( this, &Pic_module::transport_dbg );
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
Pic_module::~Pic_module( void )
{
  INFO( ALWAYS, "Destroyed " << name() );
}

//------------------------------------------------------------------------------
void Pic_module::end_of_elaboration( void )
{
  sc_assert( irq_send_port.size() <= M );
  m_source_irq.resize( Memory_map::max_irq()+1 );
  m_source_targets.resize( Memory_map::max_irq()+1 );
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Pic_module::b_transport
( tlm_payload_t& trans
, sc_time&       delay
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
Pic_module::transport_dbg
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
bool Pic_module::configure( tlm_payload_t& trans )
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
      extn->configuration.get( "addr_clocks", m_addr_clocks    );
      extn->configuration.get( "read_clocks", m_read_clocks    );
      extn->configuration.get( "write_clocks", m_write_clocks  );
      INFO( DEBUG, "Updated configuration " << m_configuration );
    }
  }

  trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
  return trans.get_command() == TLM_IGNORE_COMMAND;
}

//------------------------------------------------------------------------------
void Pic_module::execute_transaction( tlm_payload_t& trans )
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
Pic_module::nb_transport_fw
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
Pic_module::targ_peq_cb
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
Pic_module::send_end_req( tlm_payload_t& trans )
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
Pic_module::send_response( tlm_payload_t& trans )
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
Pic_module::execute_transaction_process( void )
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
bool Pic_module::payload_is_ok
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
Pic_module::transport ( tlm_payload_t& trans, sc_time& delay)
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
// Pic main actions
//------------------------------------------------------------------------------
void Pic_module::pic_thread( void )
{
  for(;;) {
    // Wait for an interrupt
    // Note: wait() will not block if there are interrupts already queued up
    irq_channel.recv_if->wait();
    string source = irq_channel.recv_if->get_next();
    INFO( MEDIUM, name() << " received interrupt"
          << " from " << source << " at "   << sc_time_stamp()
    );
    // Look up the interrupt assignment
    int iSource = Memory_map::find_irq( source );
    sc_assert( iSource >= 0 and iSource < m_source_irq.size() );

    // Set interrupt pending
    m_source_irq[ iSource ].pending = true;
    // Nothing more unless individual interrupt is enabled
    if( m_source_irq[ iSource ].enabled == false) continue;

    // Notify appropriate targets the source is configured for
    for( int iTarget=0; iTarget<irq_send_port.size(); ++iTarget ) {
      // Is this target configured to receive this interrupt?
      if( m_source_targets[ iSource ].test( iTarget ) // targeted
      and ( m_source_irq[ iSource ].priority > m_target_level[ iTarget ].front() )) // not masked
      {
        irq_send_port[ iSource ]->notify();
      }
    }
  }
}//end Pic_module::pic_thread()

//------------------------------------------------------------------------------
void Pic_module::write_actions( tlm_payload_t& trans, const sc_time& delay )
{
  Addr_t     address = trans.get_address();

  // Determine target CPU id
  Cpuid_extn* extn{trans.get_extension<Cpuid_extn>()};
  if( extn == nullptr ) REPORT( FATAL, "Transaction missing required Cpuid_extn" );
  string cpu_name = extn->name();
  int iTarget = 0;
  if( m_target_id.count( cpu_name ) == 1 ) {
    iTarget = m_target_id[ cpu_name ];
  }
  else {
    iTarget = m_target_id.size();
    m_target_id[ cpu_name ] = iTarget;
    INFO( MEDIUM, cpu_name << " added as interrupt target " << iTarget );
  }
  uint32_t iSource = m_target_select[ iTarget ];

  sc_assert( sizeof( Pic_regs_t ) == m_register_vec.size() );
  Pic_regs_t& reg{ *reinterpret_cast<Pic_regs_t*>( m_register_vec.data() ) };

  // Update from m_register_vec
  switch ( address ) {
    case /*write*/ PIC_NEXT_REG   :
    {
      // Ignored
      break;
    }
    case /*write*/ PIC_DONE_REG   :
    {
      // Clear active flag for indicated interrupt source
      if( iSource < m_source_irq.size() ) {
        m_source_irq[ iSource ].active = false;
      }
      // Restore Mask level for this target
      sc_assert( m_target_level[ iTarget ].size() > 1 );
      m_target_level[ iTarget ].pop_front();
      break;
    }
    case /*write*/ PIC_TARGET_REG :
    {
      // Set corresponding bits
      m_target_irq[ iTarget ].clearall = ( reg.target && PIC_CLEARALL_MASK ) ? 1 : 0;
      m_target_irq[ iTarget ].enabled  = ( reg.target && PIC_ENABLEIF_MASK ) ? 1 : 0;
      m_target_irq[ iTarget ].mask = reg.target && PIC_PRI_MASK;
      if( not m_target_level[ iTarget ].empty() ) {
        m_target_level[ iTarget ].pop_back();
      }
      m_target_level[ iTarget ].push_back( m_target_irq[ iTarget ].mask );

      // Clearall action
      if( m_target_irq[ iTarget ].clearall == 1 ) {
        for( auto& irq : m_source_irq ) {
          irq.pending = 0;
          irq.active = 0;
        }
      }
      break;
    }
    case /*write*/ PIC_IDENT_REG  :
    {
      // Ignored
      break;
    }
    case /*write*/ PIC_CONFIG_REG :
    {
      // Ignored
      break;
    }
    case /*write*/ PIC_SELECT_REG :
    {
      m_target_select[ iTarget ] = reg.select;
      break;
    }
    case /*write*/ PIC_SOURCE_REG :
    {
      // Set corresponding bits (except active)
      m_source_irq[ iSource ].pending = ( reg.source & PIC_PENDING_MASK ) ? 1 : 0;
      m_source_irq[ iSource ].enabled = ( reg.source & PIC_ENABLED_MASK ) ? 1 : 0;
      m_source_irq[ iSource ].priority = reg.source & PIC_PRI_MASK;
      break;
    }
    case /*write*/ PIC_TARGETS_REG:
    {
      m_source_targets[ iSource ] = std::bitset<M>( reg.targets );
      break;
    }
    default:
    {
      REPORT( FATAL, "Bad address " << address );
    }
  }//endswitch
}

//------------------------------------------------------------------------------
void Pic_module::read_actions( tlm_payload_t& trans, const sc_time& delay )
{
  Addr_t     address = trans.get_address();

  // Determine target CPU id
  Cpuid_extn* extn{trans.get_extension<Cpuid_extn>()};
  if( extn == nullptr ) REPORT( FATAL, "Transaction missing required Cpuid_extn" );
  string cpu_name = extn->name();
  int iTarget = 0;
  if( m_target_id.count( cpu_name ) == 1 ) {
    iTarget = m_target_id[ cpu_name ];
  }
  else {
    iTarget = m_target_id.size();
    m_target_id[ cpu_name ] = iTarget;
    INFO( MEDIUM, cpu_name << " added as interrupt target " << iTarget );
  }
  uint32_t iSource = m_target_select[ iTarget ];

  Pic_regs_t& reg{ *reinterpret_cast<Pic_regs_t*>( m_register_vec.data() ) };

  // Modify m_register_vec
  switch ( address ) {
    case /*read*/ PIC_NEXT_REG   :
    {
      // Find the highest priority pending interrupt for this interface that we're
      // currently able to process due to target mask level.
      int target_mask = m_target_irq[ iTarget ].mask;
      auto highest = std::make_pair( PIC_INVALID_IRQ, -1 );
      for( iSource=0; iSource<m_source_irq.size(); ++iSource ) {
        bool source_pending = ( m_source_irq[ iSource ].pending  == 1 );
        int  source_priority = m_source_irq[ iSource ].priority;
        if( source_pending and ( source_priority > target_mask ) )
        {
          if( source_priority > highest.second ) {
            highest = std::make_pair( iSource, source_priority );
          }
        }
      }
      if( highest.second < 0 ) {
        reg.next = PIC_INVALID_IRQ;
      } else {
        reg.next = highest.first;
        // Change from pending to active
        if( reg.next < PIC_INVALID_IRQ ) {
          m_source_irq[ reg.next ].active = 1;
        }
      }
      break;
    }
    case /*read*/ PIC_DONE_REG   :
    {
      // Read as zero
      reg.done = 0;
      break;
    }
    case /*read*/ PIC_TARGET_REG :
    {
      reg.target = 0;
      reg.config |= ( iTarget  << PIC_TARGETS_LSB ) & PIC_TARGETS_MASK;
      reg.target |= m_target_irq[ iTarget ].clearall ? PIC_CLEARALL_MASK : 0;
      reg.target |= m_target_irq[ iTarget ].enabled  ? PIC_ENABLEIF_MASK : 0;
      reg.target |= m_target_irq[ iTarget ].mask & PIC_PRI_MASK;
      m_target_irq[ iTarget ].clearall = 0;
      break;
    }
    case /*read*/ PIC_IDENT_REG  :
    {
      reg.ident = m_pic_identity;
      break;
    }
    case /*read*/ PIC_CONFIG_REG :
    {
      reg.config = 0;
      reg.config |= ( m_target_id.size()  << PIC_TARGETS_LSB ) & PIC_TARGETS_MASK;
      reg.config |= ( m_source_irq.size() << PIC_SOURCES_LSB ) & PIC_SOURCES_MASK;
      break;
    }
    case /*read*/ PIC_SELECT_REG :
    {
      reg.select = m_target_select[ iTarget ];
      break;
    }
    case /*read*/ PIC_SOURCE_REG :
    {
      reg.source = 0;
      reg.source |= m_source_irq[ iSource ].pending ? PIC_PENDING_MASK : 0;
      reg.source |= m_source_irq[ iSource ].active  ? PIC_ACTIVE_MASK  : 0;
      reg.source |= m_source_irq[ iSource ].enabled ? PIC_ENABLED_MASK : 0;
      reg.source |= m_source_irq[ iSource ].priority & PIC_PRI_MASK;
      break;
    }
    case /*read*/ PIC_TARGETS_REG:
    {
      reg.targets = m_source_targets[ iSource ].to_ulong();
      break;
    }
    default:
    {
      REPORT( FATAL, "Bad address " << address );
    }
  }//endswitch
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
