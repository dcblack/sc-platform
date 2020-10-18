#include "bus/memory_map.hpp"
#include "report/report.hpp"
#include "report/summary.hpp"
#include "common/common.hpp"
#include <string>
#include <list>
#include <vector>
#include <map>

using namespace std;
using namespace sc_core;
namespace {
  const char * const MSGID{ "/Doulos/Example/map_test" };
}

// Declarations
Port_t   decode_address      ( Addr_t  address, Addr_t& masked_address );
uint64_t reconstruct_address ( Addr_t address, Port_t port, bool bias_upwards );
void     build_port_map      ( void );
void     dump_port_map       ( int level );
void     check_port_map_and_update_configuration( void );

string name( string name = "" )
{
  static string m_name{ "top.cpu" };
  if( name.length() > 0 ) m_name = name;
  return m_name;
}

string kind( string kind = "" )
{
  static string m_kind{ "Cpu_name" };
  if( kind.length() > 0 ) m_kind = kind;
  return m_kind;
}

struct Command
{
  string       m_initiator;
  string       m_bus;
  string       m_target;
  string       m_yaml;
  Addr_t       m_addr;
  bool         m_processing{ true };
  list<string> m_cmds;
  Address_map m_addr_map;
  bool         processing( void ) const { return not m_processing; }
  void         process( void );
  void add( initializer_list<string> cmds ) {
    m_cmds.splice( m_cmds.end(), cmds );
  }
  size_t size( void ) const { return m_cmds.size(); }
} command;

void Command::process( void )
{
  while( not m_cmds.empty() ) {
    string cmd = m_cmds.front();
    string arg;
    if( cmd == "yaml" and m_cmds.size() > 1 ) {
      m_cmds.pop_front();
      arg = m_cmds.front();
      m_cmds.pop_front();
    } else if( cmd == "init" and m_cmds.size() > 1 ) {
      m_cmds.pop_front();
      arg = m_cmds.front();
      m_cmds.pop_front();
      NOT_YET_IMPLEMENTED();
    } else if( cmd == "targ" and m_cmds.size() > 1 ) {
      m_cmds.pop_front();
      arg = m_cmds.front();
      m_cmds.pop_front();
      NOT_YET_IMPLEMENTED();
    } else if( cmd == "bus" and m_cmds.size() > 1 ) {
      m_cmds.pop_front();
      arg = m_cmds.front();
      m_cmds.pop_front();
      NOT_YET_IMPLEMENTED();
    } else if( cmd == "findaddr" and m_cmds.size() > 1 ) {
      m_cmds.pop_front();
      arg = m_cmds.front();
      m_cmds.pop_front();
      NOT_YET_IMPLEMENTED();
    } else if( cmd == "decode" and m_cmds.size() > 1 ) {
      m_cmds.pop_front();
      arg = m_cmds.front();
      m_cmds.pop_front();
      NOT_YET_IMPLEMENTED();
    } else if( cmd == "quit" ) {
      m_cmds.pop_front();
      m_processing = false;
      break;
    } else if( m_cmds.size() == 1 ) {
      break;
    }
  }
}

//------------------------------------------------------------------------------
int sc_main( int argc, char* argv[])
{
  Addr_t address = 0x1000ull;

  if( sc_argc() < 2 ) {
    command.add(
    { "yaml", "memory_map.yaml"
    , "init", "top.cpu"
    , "targ", "top.ram"
    , "bus",  "top.bus"
    , "findaddr", "top.ram"
    , "decode", "0x1000"
    , "quit"
    });
  }
  // Scan command-line
  for( int i=1; i<sc_argc(); ++i ) {
    string arg( sc_argv()[i] );
    if( arg == "-yaml" and (i+1) < sc_argc() ) {
      arg = name(sc_argv()[++i]);
      command.add({"yaml", arg});
    } else if( arg == "-init" and (i+1) < sc_argc() ) {
      arg = name(sc_argv()[++i]);
      command.add({"init", arg});
    } else if( arg == "-bus" and (i+1) < sc_argc() ) {
      arg = name(sc_argv()[++i]);
      command.add({"bus", arg});
    } else if( arg == "-targ" and (i+1) < sc_argc() ) {
      arg = name(sc_argv()[++i]);
      command.add({"targ", arg});
    } else if( arg == "-kind" and (i+1) < sc_argc() ) {
      arg = kind(sc_argv()[++i]);
    }
  }

  // address = find_address( target );
  // m_addr_map = Memory_map::get_address_map( busname );

  if( command.size() ) {
    command.process();
  }
  while( command.processing() ) 
  {
    char line[256];
    std::cin.getline(line,256);
    istringstream is(line);
    string word;
    while( not is.eof() ) {
      is >> word;
      command.add({word});
    }
    command.process();
  }

  return Summary::report();
}//end sc_main()

//------------------------------------------------------------------------------
Addr_t find_address ( string path )
{
  INFO( DEBUG, "Searching for address from " << name() << " to " << path );
  list<string> device_path = { name(), path };
  auto target_info = Memory_map::get_target_path_info( device_path );
  sc_assert( target_info.base != BAD_ADDR );
  MESSAGE( "Found address " << STREAM_HEX << target_info.base );
  MESSAGE( " from " << name() << " to " << path );
  MEND( DEBUG );
  return target_info.base;
}

//------------------------------------------------------------------------------
Port_t decode_address ( Addr_t  address, Addr_t& masked_address )
{
  if( address == BAD_ADDR ) {
    REPORT( ERROR, "Unable to decode BAD_ADDR" );
    return BAD_PORT;
  }
  INFO( DEBUG, "Decoding address " << STREAM_HEX << address );

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
}//end decode_address()

//------------------------------------------------------------------------------
Addr_t reconstruct_address( Addr_t address, Port_t port, bool bias_upwards )
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
}//end reconstruct_address()

//------------------------------------------------------------------------------
void build_port_map( void )
{
  string  config_name;
  string  config_kind;
  Depth_t config_size;
    INFO( DEBUG, "Mapping port " << port << " " << config_name );
    size_t matches{ 0 };
    for( auto& mapping : m_addr_map ) {
      Addr_t       addr{ mapping.first  };
      Target_info& mapped{ mapping.second };
      INFO( DEBUG, "Target mapped: " << mapped );
      if( mapped.name == config_name ) {
        ++matches;
        INFO( DEBUG, "Found port match at " << STREAM_HEX << addr );
        mapped.port = port;
        mapped.kind = config_kind;
        if( config_size == UNASSIGNED ) {
          config_size = mapped.size; // should send back
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
}//end build_port_map()

void dump_port_map( int level )
{
  MESSAGE( "Port map for " << name() << ":\n" );

  for( auto rit = m_addr_map.rbegin(); rit!=m_addr_map.rend(); ++rit ) {
    const Addr_t&      addr{ rit->first  };
    const Target_info& info{ rit->second };
    MESSAGE( STREAM_HEX << "  - {"
             << " base: " << setw(10) << info.base
             << " last: " << setw(10) << info.last
             << " size: " << setw(6) << info.size
             << " port: " << setw(2) << STREAM_DEC << info.port
             << " name: " << info.name
             << " kind: " << info.kind
             << " }\n";
           );
  }

  MEND( NONE + level );
}//end dump_port_map()

//------------------------------------------------------------------------------
void check_port_map_and_update_configuration( void )
{
  dump_port_map( SC_DEBUG );
  INFO( MEDIUM, "Checking port map for " << name() );
  Addr_t  min_address{ MAX_ADDR }; // used to update bus configuration
  Addr_t  max_address{ 0 };        // used to update bus configuration
  size_t  mapping_errors  { 0 };

  // Examine each mapping for consistency and overlaps
  for( const auto& mapping : m_addr_map ) {
    const Addr_t       addr{ mapping.first  };
    const Target_info& info{ mapping.second };

    if( info.base == BAD_ADDR ) {
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

        MESSAGE( "\n  - Overlapping regions in Bus address map: " << STREAM_HEX
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
    INFO( MEDIUM, "Port map valid for " << name() );
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
