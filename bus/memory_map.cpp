#include "bus/memory_map.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//  #     # ##### #     #  ####  #####  #     #        #     #    #    #####      
//  ##   ## #     ##   ## #    # #    #  #   #         ##   ##   # #   #    #     
//  # # # # #     # # # # #    # #    #   # #          # # # #  #   #  #    #     
//  #  #  # ##### #  #  # #    # #####     #           #  #  # #     # #####      
//  #     # #     #     # #    # #  #      #           #     # ####### #          
//  #     # #     #     # #    # #   #     #           #     # #     # #          
//  #     # ##### #     #  ####  #    #    #    ###### #     # #     # #          
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
#include "utility/convert.hpp"
#include "yaml-cpp/yaml.h"
using namespace std;

namespace {
  const char * const MSGID{ "/Doulos/Example/Memory_map" };
}

std::ostream& operator<<( std::ostream& os, const Target_info& rhs )
{
  os << "{ name: " << rhs.name << " kind: " << rhs.kind;
  os << " port: " << STREAM_DEC;
  if( rhs.port == BAD_PORT ) os << "BAD_PORT";
  else                       os << rhs.port;
  os << " base: " << STREAM_HEX;
  if( rhs.base == BAD_ADDR ) os << "BADD_ADDR";
  else                       os << rhs.base;
  os << " last: " << rhs.last
     << " size: " << rhs.size;
  os << " irq: ";
  if( rhs.irq == UNASSIGNED ) os << "UNASSIGNED";
  else                        os << rhs.irq;
  os << " }";
  return os;
}

//------------------------------------------------------------------------------
Memory_map& Memory_map::instance( void )
{
  static Memory_map the_map;
  return the_map;
}

//------------------------------------------------------------------------------
Memory_map::Origin_map& Memory_map::origin_map( void ) {
  return instance().m_origin_map;
}

namespace {
void dump_path( int lineno, const list<string>& device_path )
{
  MESSAGE( __FILE__ << ":" << STREAM_DEC << lineno << "\ndevice_path =" );
  for( const auto& device : device_path ) MESSAGE( " " << device );
  MEND( DEBUG );
}
}

//------------------------------------------------------------------------------
Addr_t Memory_map::find_address ( string from_initiator, string to_target )
{
  INFO( DEBUG, "Searching for address from " << from_initiator << " to " << to_target );
  list<string> device_path = { from_initiator, to_target };
  auto target_info = find_target_info( device_path );
  sc_assert( target_info.base != BAD_ADDR );
  MESSAGE( "Found address " << STREAM_HEX << target_info.base );
  MESSAGE( " from " << from_initiator << " to " << to_target );
  MEND( DEBUG );
  return target_info.base;
}

//------------------------------------------------------------------------------
/*static*/ int Memory_map::max_irq( void )
{
  if( origin_map().empty() ) {
    instance().read_yaml();
  }
  return instance().m_max_irq;
}

//------------------------------------------------------------------------------
/*static*/ int Memory_map::find_irq( std::string target )
{
  if ( instance().m_target_ptr_map.count( target ) != 0 ) {
    return instance().m_target_ptr_map[ target ]->irq;
  }
  else {
    return UNASSIGNED;
  }
}

//------------------------------------------------------------------------------
/*static*/ Target_info Memory_map::find_target_info( list<string>& device_path )
{
  sc_assert( not device_path.empty() );
  dump_path( __LINE__, device_path );

  if ( instance().m_origin_map.empty() )
  {
    instance().read_yaml();
  }

  Target_info result;
  result.base = 0;
  string start = device_path.front();
  device_path.pop_front();
  string from = start;

  while ( not device_path.empty() )
  {
    string to = device_path.front();
    device_path.pop_front();
    sc_assert( to.length() > 0 );

    if ( instance().m_origin_map.count( from ) == 0 ) {
      REPORT( WARNING, "No origin named '" << from << "' in memory map!" );
      return result; // empty handed
    }

    INFO( DEBUG, "Searching from " << from << " to " << to );

    // Does this origin have a direct hit?
    if ( instance().m_origin_map[from].target_map.count( to ) == 1 ) {
      INFO( DEBUG, "Found " << to );
      Addr_t offset = result.base;
      result = instance().m_origin_map[from].target_map[to];
      result.base += offset;
      from = to; //< in case this is just an intermediate point
      dump_path( __LINE__, device_path );
      continue;
    }

    // Search routers
    for ( const auto& target : instance().m_origin_map[from].target_map ) {
      INFO( DEBUG, "Examining target " << target.first );

      if ( instance().m_origin_map[ target.first ].category == Category::router ) {
        INFO( DEBUG, "Searching router " << target.first );
        device_path.push_front( to );
        device_path.push_front( target.first );
        dump_path( __LINE__, device_path );
        Addr_t offset = instance().m_origin_map[ target.first ].base;
        result = find_target_info( device_path );
        INFO( DEBUG, "\nresult = " << result );

        if ( result.base != BAD_ADDR ) {
          result.base += offset;
          goto DONE; // we found it!
        }
      }
    }

    dump_path( __LINE__, device_path );
  }//endforeach
  DONE:

  if ( result.base == BAD_ADDR )
  {
    REPORT( ERROR, "Path not found from " << start );
  }

  return result;
}//end Memory_map::find_target_info()

//------------------------------------------------------------------------------
/*static*/ Address_map Memory_map::get_address_map( const string& origin_name )
{
  Address_map the_address_map;
  if( instance().m_origin_map.empty() ) {
    instance().read_yaml();
  }
  if( instance().m_origin_map.count( origin_name ) == 0 ) {
    REPORT( FATAL, "No origin named '" << origin_name << "' in memory map!" );
    return the_address_map; // empty handed
  }
  for( const auto& entry : instance().m_origin_map[ origin_name ].target_map ) {
    Addr_t base = entry.second.base;
    if( the_address_map.count( base ) != 0 ) { // unique
      REPORT( ERROR, "\n- Duplicated address " << STREAM_HEX << base << " in memory map for " << origin_name
                  << ".\n- Duplicate is " << entry.first
                  << ".\n- Orignal was " << the_address_map[ base ].name
                  << "."
            );
    }
    the_address_map[ base ] = entry.second;
  }
  return the_address_map;
}//end Memory_map::get_address_map()

//------------------------------------------------------------------------------
/*static*/ void Memory_map::read_yaml( void )
{
  YAML::Node root;
  INFO( DEBUG, "Reading YAML" );
  try
  {
    root = YAML::LoadFile( instance().m_yaml_filename );
  }
  catch ( const YAML::Exception& e )
  {
    REPORT( FATAL, e.what() );
  }

  // Add YAML origin instances to the map
  INFO( DEBUG, "Processing YAML" );
  for ( const auto& yaml_origin : root )
  {
    // Add origin entry to map
    string origin_name = yaml_origin.first.as<string>();
    instance().m_origin_map[ origin_name ] = Origin_info();
    Origin_info& origin_info{ instance().m_origin_map[ origin_name ] };
    origin_info.name = origin_name;

    if ( yaml_origin.second["type"] ) {
      string category{ yaml_origin.second["type"].as<string>() };

      if ( category == "initiator" ) {
        origin_info.category = Category::initiator;
      }
      else if ( category == "interconnect" ) {
        origin_info.category = Category::interconnect;
      }
      else if ( category == "router" ) {
        origin_info.category = Category::router;
      }
      else if ( category == "target" ) {
        origin_info.category = Category::target;
      }
      else {
        origin_info.category = Category::unknown;
      }
    }

    if ( yaml_origin.second["addr"] ) {
      INFO( DEBUG+1, "Reading origin address" );
      to_int( origin_info.base, yaml_origin.second["addr"].as<string>() );
    }

    if ( yaml_origin.second["size"] ) {
      INFO( DEBUG+1, "Reading origin size" );
      to_int( origin_info.size, yaml_origin.second["size"].as<string>() );
    }

    // Add each YAML target to the map
    INFO( DEBUG, "Processing origin: " << origin_name );
    for ( const auto& yaml_target : yaml_origin.second["target"] ) {
      // Add target entry to origin
      string target_name = yaml_target.first.as<string>();
      origin_info.target_map[ target_name ] = Target_info();
      // Add to target reference
      if ( instance().m_target_ptr_map.count( target_name ) == 0 ) {
        instance().m_target_ptr_map[ target_name ] = &(origin_info.target_map[ target_name ]);
      }
//    else {
//      REPORT( WARNING, "Target '" << target_name << "' duplicate ignored from "
//                    << "origin '" << origin_name << "'." );
//    }
      Target_info& target_info{ origin_info.target_map[ target_name ] };
      target_info.name = target_name;
      INFO( DEBUG, "Processing target: " << target_name );

      // Add each field if present
      if ( yaml_target.second["addr"] ) {
        INFO( DEBUG+1, "Reading target address" );
        to_int( target_info.base, yaml_target.second["addr"].as<string>() );
      }
      else {
        REPORT( WARNING, "Missing base address for target " << target_name );
      }

      if ( yaml_target.second["size"] ) {
        INFO( DEBUG+1, "Reading target size" );
        to_int( target_info.size, yaml_target.second["size"].as<string>() );
      }
      //else optional

      if ( yaml_target.second["irq"] ) {
        INFO( DEBUG+1, "Reading target irq" );
        to_int( target_info.irq, yaml_target.second["irq"].as<string>() );
        if( instance().m_max_irq < target_info.irq) instance().m_max_irq = target_info.irq;
      }
      //else optional

      if ( yaml_target.second["kind"] ) {
        INFO( DEBUG+1, "Reading target kind" );
        target_info.kind = yaml_target.second["kind"].as<string>();
      }
      else {
        REPORT( WARNING, "Missing kind for target " << target_name );
      }

      if ( target_info.size != UNASSIGNED ) {
        target_info.last = target_info.base + target_info.size - 1;
      }
    }//endforeach yaml_target
  }//endforeach yaml_origin
  INFO( DEBUG, "Done reading YAML\n" );
}//end Memory_map::read_yaml()

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
