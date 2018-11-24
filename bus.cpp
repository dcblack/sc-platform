////////////////////////////////////////////////////////////////////////////////
//
//  #####  #    #  ####         #     # ####   #                                 
//  #    # #    # #    #        ##   ## #   #  #                                 
//  #    # #    # #             # # # # #    # #                                 
//  #####  #    #  ####         #  #  # #    # #                                 
//  #    # #    #      #        #     # #    # #                                 
//  #    # #    # #    #        #     # #   #  #                                 
//  #####   ####   ####  ###### #     # ####   #####                             
//
////////////////////////////////////////////////////////////////////////////////
#include "bus.hpp"
#include "report.hpp"
#include "config_extn.hpp"
#include "route_extn.hpp"
#include <memory>
namespace {
  const char* MSGID{ "/Doulos/Example/TLM-bus" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

#include <tlm_utils/simple_target_socket.h>

//------------------------------------------------------------------------------
// Constructor
Bus_module::Bus_module( sc_module_name instance_name )
  : m_mm       { Memory_manager<>::instance() }
{
  targ_socket.register_b_transport( this, &Bus_module::b_transport );
  targ_socket.register_nb_transport_fw( this, &Bus_module::nb_transport_fw );
  targ_socket.register_get_direct_mem_ptr( this, &Bus_module::get_direct_mem_ptr );
  targ_socket.register_transport_dbg( this, &Bus_module::transport_dbg );
  init_socket.register_nb_transport_bw( this, &Bus_module::nb_transport_bw );
  init_socket.register_invalidate_direct_mem_ptr( this, &Bus_module::invalidate_direct_mem_ptr );
}

//------------------------------------------------------------------------------
// Destructor
Bus_module::~Bus_module( void )
{
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Bus_module::b_transport( int id, tlm_payload_t& trans, sc_time& delay )
{
  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );

  if( target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    return;
  }

  INFO( DEBUG+1, name() << " forwarding to port " << target << " with address " << masked_address );
  trans.set_address( masked_address );
  init_socket[target]->b_transport( trans, delay );
}

//------------------------------------------------------------------------------
tlm_sync_enum
Bus_module::nb_transport_fw
( int id
  , tlm_payload_t& trans
  , tlm_phase& phase
  , sc_time& delay
)
{
  Route_extn* route_extn;
  if( phase == tlm::BEGIN_REQ )
  {
    route_extn = new Route_extn;
    route_extn->id = id;
    m_accessor( trans ).set_extension( route_extn );
  }

  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );

  if( target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    if( route_extn != nullptr ) {
      m_accessor( trans ).clear_extension( route_extn );
      delete route_extn;
    }
    return TLM_COMPLETED;
  }

  trans.set_address( masked_address );

  tlm_sync_enum status = init_socket[target]->nb_transport_fw( trans, phase, delay );

  if( status == tlm::TLM_COMPLETED and route_extn != nullptr ) {
    m_accessor( trans ).clear_extension( route_extn );
    delete route_extn;
  }

  return status;
}

//------------------------------------------------------------------------------
bool
Bus_module::get_direct_mem_ptr
( int id
  , tlm_payload_t& trans
  , tlm_dmi& dmi_data
)
{
  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );

  if(  target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    return false;
  }

  trans.set_address( masked_address );

  bool status = init_socket[target]->get_direct_mem_ptr( trans, dmi_data );

  dmi_data.set_start_address( reconstruct_address( dmi_data.get_start_address(), id ) );
  dmi_data.set_end_address( reconstruct_address( dmi_data.get_end_address(),   id ) );

  return status;
}

//------------------------------------------------------------------------------
unsigned int
Bus_module::transport_dbg( int id, tlm_payload_t& trans )
{
  uint64 address{ trans.get_address() };
  bool config_only{config( trans )};

  if(  config_only and address == MAX_ADDR ) {
    trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
    trans.set_response_status( TLM_OK_RESPONSE );
    return 0;
  }

  uint64 masked_address;
  unsigned int target = decode_address( address, masked_address );

  if(  target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    return 0;
  }

  trans.set_address( masked_address );

  return init_socket[target]->transport_dbg( trans );
}

////////////////////////////////////////////////////////////////////////////////
// Backward interface

//------------------------------------------------------------------------------
tlm_sync_enum
Bus_module::nb_transport_bw
( int            id
, tlm_payload_t& trans
, tlm_phase&     phase
, sc_time&       delay
)
{
  Route_extn* route_extn;
  m_accessor( trans ).get_extension( route_extn );
  sc_assert( route_extn != nullptr );
  tlm::tlm_sync_enum status = targ_socket[ route_extn->id ]->nb_transport_bw( trans, phase, delay );
  if( status == tlm::TLM_COMPLETED )
  {
    m_accessor( trans ).clear_extension( route_extn );
    delete route_extn;
  }
  return status;
}

//------------------------------------------------------------------------------
void
Bus_module::invalidate_direct_mem_ptr
( int id
  , uint64 start_range
  , uint64 end_range
)
{
  // Reconstruct address range in system memory map
  sc_dt::uint64 bw_start_range = reconstruct_address( start_range, id );
  sc_dt::uint64 bw_end_range   = reconstruct_address( end_range,   id );

  // Propagate call backward to all initiators
  for ( unsigned int i = 0; i < targ_socket.size(); i++ ) {
    targ_socket[i]->invalidate_direct_mem_ptr( bw_start_range, bw_end_range );
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helpers

#include "memory_map.hpp" //< simple enumerated constants

bool
Bus_module::mask_if_fits( Addr_t& address, Addr_t start, Depth_t depth ) const
{
  INFO( DEBUG+1, "Testing address " << HEX << address << " against " << start << ".." << ( start+depth ) );
  if( ( address >= start ) and ( address < ( start + depth ) ) ) {
    address -= start;
    INFO( DEBUG+1, "Matched! Returning masked address " << HEX << address );
    return true;
  }
  else {
    INFO( DEBUG+1, "No match." );
    return false;
  }
}

//------------------------------------------------------------------------------
// Return true if configuration is all that is needed
bool Bus_module::config
( tlm_payload_t& trans
)
{
  //sc_assert( trans.get_address() == MAX_ADDR );
  Config_extn* config_extn{trans.get_extension<Config_extn>()};

  if( config_extn != nullptr ) {
    // Ensure we have the correct name locally
    m_config.set( "name", string( name() ) );
    m_config.set( "kind", string( kind() ) );

    if( config_extn->config.empty() ) {
      config_extn->config = m_config;
    }
    else {
      m_config.update( config_extn->config );
    }
  }

  return trans.get_command() == TLM_IGNORE_COMMAND;
}

//------------------------------------------------------------------------------
unsigned int //< port index
Bus_module::decode_address
( Addr_t  address,
  Addr_t& masked_address
)
{
  INFO( DEBUG+1, "Decoding address " << HEX << address );
  masked_address = address;
#ifndef CRUDE

  // Probing
  if( m_port_vec.empty() ) {
    build_port_map();
  }

  // Lookup for appropriate start, depth
  for ( int port = 0; port < init_socket.size(); ++port ) {
    if( mask_if_fits( masked_address, m_port_vec[port].start, m_port_vec[port].depth ) ) {
      return m_port_vec[port].port;
    }
  }
  REPORT( WARNING, "No address => port match found!" );

  return ~0;
#else
  // Crude - only allows for fixed one layer North bus
  if( mask_if_fits( masked_address, ROM_BASE, ROM_DEPTH ) ) {
    return ROM_PORT;
  }

  if( mask_if_fits( masked_address, RAM_BASE, RAM_DEPTH ) ) {
    return RAM_PORT;
  }

  return ~0;
#endif
}

//------------------------------------------------------------------------------
uint64_t // address
Bus_module::reconstruct_address
( Addr_t address
, int    id
)
{
#ifndef CRUDE
  Addr_t  start = m_port_vec[id].start;
  Depth_t depth = m_port_vec[id].depth;
  Addr_t  reconstructed{ start + address };

  // Clip as required
  if( reconstructed >= ( start + depth ) ) {
    reconstructed = start + depth - 1;
  }

  return reconstructed;
#else

  // Crude
  if( id == ROM_PORT ) {
    return ROM_BASE + address;
  }

  if( id == RAM_PORT ) {
    return RAM_BASE + address;
  }

#endif
  return MAX_ADDR;
}

//------------------------------------------------------------------------------
void
Bus_module::build_port_map( void )
{
  INFO( MEDIUM, "Building port map for " << name() );

  // Create a transaction for probing
  tlm_payload_t& trans{ *m_mm.allocate_acquire() };
  trans.set_command( TLM_IGNORE_COMMAND );
  // Add sticky extension as needed
  Config_extn* config_extn{ trans.get_extension<Config_extn>() };

  if( config_extn == nullptr ) {
    config_extn = new Config_extn;
    trans.set_extension( config_extn );
  }
  else {
    config_extn->config.set_defaults();
  }

  // Setup port mapping vector
  m_port_vec.resize( init_socket.size() );

  // Foreach initiator port
  for ( int port = 0; port < init_socket.size(); ++port ) {
    // Initialize fields that need refreshing on every transaction
    trans.set_address( MAX_ADDR );
    trans.set_response_status( TLM_INCOMPLETE_RESPONSE );
    trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
    // Make sure data is cleared to compel receiver to fill
    config_extn->config.clear_data();
    int count = init_socket[port]->transport_dbg( trans );
    INFO( DEBUG, "transport_db response: " << trans.get_response_string() );
    NOINFO( DEBUG, "Got " << config_extn->config );

    if( trans.get_response_status() == TLM_OK_RESPONSE ) {
      if( config_extn->config.has_key( "target_start" )
           and config_extn->config.has_key( "target_depth" )
           and config_extn->config.has_key( "name" )
           and config_extn->config.has_key( "kind" )
         ) {
        m_port_vec[port].port = port;
        config_extn->config.get( "name",         m_port_vec[port].name );
        config_extn->config.get( "kind",         m_port_vec[port].kind );
        config_extn->config.get( "target_start", m_port_vec[port].start );
        config_extn->config.get( "target_depth", m_port_vec[port].depth );
      }
    }
  }//endfor port

  config_extn->config.clear_data();
  trans.release();
  check_port_map();
}

//------------------------------------------------------------------------------
void
Bus_module::dump_port_map( int level )
{
  MESSAGE( "Port map for " << name() << ":" );

  for ( int port = 0; port < init_socket.size(); ++port ) {
    MESSAGE( "\n   port: " << port
             << "\n   - name:  " << m_port_vec[port].name
             << "\n   - kind:  " << m_port_vec[port].kind
             << HEX
             << "\n   - start: " << m_port_vec[port].start
             << "\n   - depth: " << m_port_vec[port].depth
           );
  }

  MEND( NONE + level );
}

//------------------------------------------------------------------------------
void
Bus_module::check_port_map( void )
{
  dump_port_map( SC_DEBUG );
  INFO( MEDIUM, "Checking port map for " << name() );
  Addr_t  min_addr{ MAX_ADDR };
  Addr_t  max_addr{ 0 };
  size_t  errors  { 0 };

  for ( int port = 0; port < init_socket.size(); ++port ) {
    Addr_t  start = m_port_vec[port].start;
    Depth_t depth = m_port_vec[port].depth;

    if( start == MAX_ADDR ) {
      if( errors++ == 0 ) {
        MESSAGE( "Port map errors detected:" );
      }

      MESSAGE( "\n  - Port " << port << " from "
               << m_port_vec[port].kind << " " << m_port_vec[port].name << " "
               << "missing mapping" );
      continue;
    }

    if( start + depth < start ) {
      if( errors++ == 0 ) {
        MESSAGE( "Port map errors detected:" );
      }

      MESSAGE( "\n  - Port " << port << " from "
               << m_port_vec[port].kind << " " << m_port_vec[port].name << " "
               << "address range wraps around 64 bits!" );
      continue;
    }

    if( start < min_addr ) {
      min_addr = start;
    }

    if( start + depth > max_addr ) {
      max_addr = start + depth;
    }

    // Check for overlapping address ranges
    for ( int prev = 0; prev < port; ++prev ) {
      if( m_port_vec[prev].start == MAX_ADDR ) {
        continue;
      }

      if( start < ( m_port_vec[prev].start + m_port_vec[prev].depth )
           and m_port_vec[prev].start < ( start + depth ) ) {
        if( errors++ == 0 ) {
          MESSAGE( "Port map errors detected:" );
        }

        MESSAGE( "\n  - Overlapping regions in Bus address map: " << HEX
                 << m_port_vec[port].kind << " " << m_port_vec[port].name << " "
                 << start << ".." << m_port_vec[port].start + depth - 1
                 << " and "
                 << m_port_vec[prev].kind << " " << m_port_vec[prev].name << " "
                 << m_port_vec[prev].start << ".." << m_port_vec[prev].start + m_port_vec[prev].depth - 1
               );
      }
    }//endfor prev
  }//endfor port

  if( errors > 0 ) {
    REPORT( ERROR, "\n\nTotal of " << errors << " detected mapping errors." );
  }
  else {
    INFO( MEDIUM, "Port map valid for " << name() );
  }

  m_config.set( "name", string( name() ) );
  m_config.set( "kind", string( kind() ) );
  m_config.set( "object_ptr", uintptr_t( this ) );
  m_config.set( "target_start", min_addr );
  m_config.set( "target_depth", Depth_t( max_addr - min_addr ) );
  INFO( DEBUG, "Bus config:\n" << m_config );
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
