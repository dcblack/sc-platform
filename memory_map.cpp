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
#include "memory_map.hpp"
#include "report.hpp"
#include "convert.hpp"
#include "yaml-cpp/yaml.h"
using namespace std;

namespace {
  const char * const MSGID{ "/Doulos/example/Memory_map" };
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

//------------------------------------------------------------------------------
/*static*/ Memory_map::Target_ptr
Memory_map::target_path( forward_list<string> device_path )
{
  sc_assert( not device_path.empty() );
  if( instance().m_origin_map.empty() ) {
    instance().read_yaml();
  }
  Target_ptr result{ new Target_info };
  int n{0}; //< count entries to validate request
  for( string from; not device_path.empty(); device_path.pop_front() ) {
    n++;
    string to { device_path.front() };
    sc_assert( to.length() > 0 );
    if( from.length() == 0 ) {
      from = to;
      continue;
    }
    if( instance().m_origin_map.count( from ) == 0 ) {
      REPORT( WARNING, "No origin named '" << from << "' in memory map!" );
      return std::unique_ptr<Target_info>(); // empty handed
    }
    // Does this origin have a direct hit?
    if( instance().m_origin_map[from].target_map.count(to) == 1 ) {
      Addr_t offset = result->base;
      *result = instance().m_origin_map[from].target_map[to];
      result->base += offset;
      from = to;
      continue;
    }
    // Search routers
    for ( const auto& target : instance().m_origin_map[from].target_map )
    {
      if ( instance().m_origin_map.count( target.first ) == 1
           and instance().m_origin_map[ target.first ].category == Category::router ) {
        result = target_path( device_path );

        if ( result->base != BAD_ADDR ) {
          break;
        }
      }
    }
  }//endforeach
  sc_assert( n > 1 );
  return result;
}//end Memory_map::target_path()

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
      REPORT( ERROR, "\n- Duplicated address " << HEX << base << " in memory map for " << origin_name
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
      INFO( DEBUG, "Reading origin address" );
      to_int( origin_info.base, yaml_origin.second["addr"].as<string>() );
    }

    if ( yaml_origin.second["size"] ) {
      INFO( DEBUG, "Reading origin size" );
      to_int( origin_info.size, yaml_origin.second["size"].as<string>() );
    }

    // Add each YAML target to the map
    INFO( DEBUG, "Processing origin: " << origin_name );
    for ( const auto& yaml_target : yaml_origin.second["target"] ) {
      // Add target entry to origin
      string target_name = yaml_target.first.as<string>();
      origin_info.target_map[ target_name ] = Target_info();
      Target_info& target_info{ origin_info.target_map[ target_name ] };
      target_info.name = target_name;
      INFO( DEBUG, "Processing target: " << target_name );

      // Add each field if present
      if ( yaml_target.second["addr"] ) {
        INFO( DEBUG, "Reading target address" );
        to_int( target_info.base, yaml_target.second["addr"].as<string>() );
      }
      else {
        REPORT( WARNING, "Missing base address for target " << target_name );
      }

      if ( yaml_target.second["size"] ) {
        INFO( DEBUG, "Reading target size" );
        to_int( target_info.size, yaml_target.second["size"].as<string>() );
      }
      //else optional

      if ( yaml_target.second["irq"] ) {
        INFO( DEBUG, "Reading target irq" );
        to_int( target_info.irq, yaml_target.second["irq"].as<string>() );
      }
      //else optional

      if ( yaml_target.second["kind"] ) {
        INFO( DEBUG, "Reading target kind" );
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
}//end Memory_map::read_yaml()

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
