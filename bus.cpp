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
#include "options.hpp"
#include <memory>
#include <tuple>
#include <map>

namespace {
  char const * const MSGID{ "/Doulos/Example/TLM-bus" };
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

  dmi_data.set_start_address( reconstruct_address( dmi_data.get_start_address(), id, false ) );
  dmi_data.set_end_address  ( reconstruct_address( dmi_data.get_end_address(),   id, true  ) );

  return status;
}

//------------------------------------------------------------------------------
unsigned int
Bus_module::transport_dbg( int id, tlm_payload_t& trans )
{
  uint64 address{ trans.get_address() };

  if(  configure( trans ) and address == BAD_ADDR ) {
    // The only purpose for this transport_dbg was to obtain the configuration
    // information for the caller.
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
  sc_dt::uint64 bw_start_range = reconstruct_address( start_range, id, false );
  sc_dt::uint64 bw_end_range   = reconstruct_address( end_range,   id, true  );

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
bool Bus_module::configure
( tlm_payload_t& trans
)
{
  //sc_assert( trans.get_address() == MAX_ADDR );
  Config_extn* config_extn{trans.get_extension<Config_extn>()};

  if( config_extn != nullptr ) {
    // Ensure we have the correct name locally
    m_configuration.set( "name", string( name() ) );
    m_configuration.set( "kind", string( kind() ) );

    if( config_extn->configuration.empty() ) {
      config_extn->configuration = m_configuration;
    }
    else {
      m_configuration.update( config_extn->configuration );
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
  if( address == BAD_ADDR ) {
    REPORT( ERROR, "Unable to decode BAD_ADDR" );
    return BAD_PORT;
  }
  INFO( DEBUG, "Decoding address " << HEX << address );

  // In case of failure
  Addr_t base = BAD_ADDR;
  masked_address = BAD_ADDR;

  // Lookup for appropriate start, depth
  auto lookup = m_addr_map.lower_bound( address );
  if( lookup == m_addr_map.end() ) {
    REPORT( WARNING, "No address => port match found!" );
    return BAD_PORT;
  }
  base = lookup->first;
  if( base == MAX_ADDR || base == BAD_ADDR ) {
    REPORT( WARNING, "No address => port match found! Unintialized entry." );
    return BAD_PORT;
  }
  if( address > lookup->second.last ) {
    REPORT( WARNING, "No address => port match found. Goes beyond closest port's maximum." );
    return BAD_PORT;
  }
  masked_address = address - base;
  return lookup->second.port;
}//end Bus_module::decode_address

//------------------------------------------------------------------------------
uint64_t // address
Bus_module::reconstruct_address
( Addr_t address
, Port_t port
, bool bias_upwards
)
{
  Addr_t base{ BAD_ADDR };
  Addr_t last;
  Addr_t min_address{ MAX_ADDR};
  Addr_t max_address{ 0 };
  Addr_t reconstructed{ BAD_ADDR };
  for( const auto& mapping : m_addr_map ) {
    if( mapping.second.port != port ) continue;
    base = mapping.second.base;
    last = mapping.second.last;
    if( base < min_address ) min_address = base;
    if( last > max_address ) max_address = last;
    if( base <= address and address <= last ) {
      // Exact match!
      reconstructed = base + address;
      break;
    }
  }
  if( reconstructed == BAD_ADDR ) {
    if( address < max_address ) address = max_address;
    if( bias_upwards ) {
    }
    else {
    }
  }

  return reconstructed;
}

void Bus_module::start_of_simulation( void )
{
  if( m_addr_map.empty() ) {
    build_port_map();
  }
}

//------------------------------------------------------------------------------
void
Bus_module::build_port_map( void )
{
  INFO( HIGH, "Building port map for " << name() );
  // Get memory map configuration
  m_addr_map = Memory_map::get_address_map( name() );

  //----------------------------------------------------------------------------
  // Get port info by probing each target for its configuration
  //----------------------------------------------------------------------------
  // Create a transaction for probing
  tlm_payload_t& probe_trans{ *m_mm.allocate_acquire_and_set(TLM_IGNORE_COMMAND) };
  // Add/reset configuration extension
  Config_extn* config_extn{ probe_trans.get_extension<Config_extn>() };
  if( config_extn == nullptr ) {
    config_extn = new Config_extn;
    probe_trans.set_extension( config_extn );
  }
  else {
    config_extn->configuration.set_defaults();
  }

  for ( int port = 0; port < init_socket.size(); ++port ) {
    // Initialize fields that need refreshing on every transaction
    probe_trans.set_address( BAD_ADDR );
    probe_trans.set_response_status( TLM_INCOMPLETE_RESPONSE );
    probe_trans.set_gp_option( TLM_FULL_PAYLOAD );
    // Make sure data is cleared to compel receiver to fill
    config_extn->configuration.clear_data();
    INFO( DEBUG, "Probing port " << port << " from " << name() );
    int count = init_socket[port]->transport_dbg( probe_trans );
    INFO( DEBUG, "Got\n" << config_extn->configuration );

    if( probe_trans.get_response_status() == TLM_OK_RESPONSE ) {
      if( config_extn->configuration.has_key( "target_size" )
      and config_extn->configuration.has_key( "name" )
      and config_extn->configuration.has_key( "kind" )
      ) {
        string  config_name;
        string  config_kind;
        Depth_t config_size;
        config_extn->configuration.get("name", config_name);
        config_extn->configuration.get("kind", config_kind);
        config_extn->configuration.get("target_size", config_size);
        // For each mapping that matches the name, we update the
        // port, kind, and optionally the size if not yet specified.
        size_t matches{ 0 };
        for( auto& mapping : m_addr_map ) {
          Addr_t       addr{ mapping.first  };
          Target_info& mapped{ mapping.second };
          if( mapped.name == config_name ) {
            ++matches;
            INFO( DEBUG, "Found port match at " << HEX << addr );
            mapped.port = port;
            mapped.kind = config_kind;
            if( config_size == UNASSIGNED ) {
              config_size = mapped.size; // {:TBD:should send back:}
            } else {
              mapped.size = config_size;
            }
            if( config_size == 0 ) {
              REPORT( ERROR, "Configured size of " << config_name << " is zero!" );
            }
            mapped.last = mapped.base + config_size - 1;
          }
        }//endfor
        if( matches == 0 ) {
          REPORT( ERROR, "No matches on port " << config_name << " => connectivity mismatch!" );
        }
      }
    }
    else {
      REPORT( ERROR, "Bad response on port " << port
                  << " " << probe_trans.get_response_string()
            );
    }
  }//endfor port

  config_extn->configuration.clear_data();
  probe_trans.release();
  check_port_map_and_update_configuration();
}//end Bus_module::build_port_map()

//------------------------------------------------------------------------------
void
Bus_module::dump_port_map( int level )
{
  MESSAGE( "\nPort map for " << name() << ":\n" );

  // Iterate backwards over map to get ascending addresses
  for( auto rit = m_addr_map.rbegin(); rit!=m_addr_map.rend(); ++rit ) {
    const Addr_t&      addr{ rit->first  };
    const Target_info& info{ rit->second };
    if( info.port == BAD_PORT and not Options::has_flag("-v") ) continue;
    MESSAGE( HEX << "  - {"
             << " base: " << setw(10) << info.base
             << " last: " << setw(10) << info.last
             << " size: " << setw(6) << info.size
             << " port: " << setw(2) << DEC << info.port
             << " name: " << info.name
             << " kind: " << info.kind
             << " }\n";
           );
  }

  MEND( NONE + level );
}

//------------------------------------------------------------------------------
void
Bus_module::check_port_map_and_update_configuration( void )
{
  dump_port_map( SC_DEBUG );
  INFO( MEDIUM+1, "Checking port map for " << name() );
  Addr_t  min_address{ MAX_ADDR }; // used to update bus configuration
  Addr_t  max_address{ 0 };        // used to update bus configuration
  size_t  mapping_errors  { 0 };

  // Examine each mapping for consistency and overlaps
  for( const auto& mapping : m_addr_map ) {
    const Addr_t       addr{ mapping.first  };
    const Target_info& info{ mapping.second };

    if( info.base == BAD_ADDR or info.base == MAX_ADDR ) {
      if( mapping_errors++ == 0 ) {
        MESSAGE( "Port map errors detected:" );
      }

      MESSAGE( "\n  - Address " << addr << " from "
               << info.kind << " " << info.name << " "
               << "doesn't match contained address" );
      continue;
    }

    if( info.last < info.base ) {
      if( mapping_errors++ == 0 ) {
        MESSAGE( "Port map errors detected:" );
      }

      MESSAGE( "\n  - Address " << addr << " from "
               << info.kind << " " << info.name << " "
               << "address range wraps around 64 bits!" );
      continue;
    }

    if( info.base < min_address ) {
      min_address = info.base;
    }

    if( info.last > max_address ) {
      max_address = info.last;
    }

    // Check for overlapping address ranges
    for( const auto& next_mapping : m_addr_map ) {
      const Addr_t       next_addr{ next_mapping.first  };
      const Target_info& next_info{ next_mapping.second };

      // Don't compare what we've seen so far
      if( next_addr <= addr ) {
        continue;
      }

      if( next_addr <= info.last ) {
        if( mapping_errors++ == 0 ) {
          MESSAGE( "Port map errors detected:" );
        }

        MESSAGE( "\n  - Overlapping regions in Bus address map: " << HEX
                 << info.kind << " " << info.name << " "
                 << addr << ".." << info.last
                 << " and "
                 << next_info.kind << " " << next_info.name << " "
                 << next_info.base << ".." << next_info.last
        );
      }
    }//endforeach next_mapping
  }//endforeach mapping

  if( mapping_errors > 0 ) {
    REPORT( ERROR, "\n\nTotal of " << mapping_errors << " mapping errors detected for " << name());
  }
  else {
    INFO( MEDIUM+1, "Port map valid for " << name() );
  }

  // Update our own configuration data
  m_configuration.set( "name", string( name() ) );
  m_configuration.set( "kind", string( kind() ) );
  m_configuration.set( "object_ptr", uintptr_t( this ) );
  m_configuration.set( "target_base", min_address );
  m_configuration.set( "target_size", Depth_t( max_address - min_address ) );
  INFO( DEBUG, "Bus configuration:\n" << m_configuration );

}//end check_port_map_and_update_configuration()

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
