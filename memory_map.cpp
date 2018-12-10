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

namespace {
  const char * const MSGID{ "/Doulos/example/Memory_map" };
}

//------------------------------------------------------------------------------
Target_ptr target_path( initializer_list<string> device_path )
{
  if( m_origin_map.empty() ) {
    Memory_map::read_yaml();
  }
  std::unique_ptr<Target_info> result{ new Target_info };
  string from;
  for( const auto& to : device_path ) {
    sc_assert( to.size() > 0 );
    if( from.size() == 0 ) {
      from = to;
      continue;
    }
    if( m_origin_map.count( from ) == 0 ) {
      REPORT( WARNING, "No origin named '" << from << "' in memory map!" );
      return std::unique_ptr<Target_info>(); // empty handed
    }
    if( m_origin_map[from].target_map.count(to) == 1 ) {
      Addr_t offset = result->base;
      *result = m_origin_map[from].target_map[to];
      result->base += offset;
      from = to;
      continue;
    }
  }//endforeach
  return result;
}

//------------------------------------------------------------------------------
Address_map Memory_map::get_address_map( const string& origin_name )
{
  Address_map the_address_map;
  if( m_origin_map.empty() ) {
    Memory_map::read_yaml();
  }
  if( m_origin_map.count( origin_name ) == 0 ) {
    REPORT( FATAL, "No origin named '" << origin_name << "' in memory map!" );
    return the_address_map; // empty handed
  }
  for( const auto& entry : m_origin_map[ origin_name ].target_map ) {
    Addr_t base = entry.second.base;
    sc_assert( the_address_map.count( base ) == 0 ); // unique
    the_address_map[ base ] = entry.second;
  }
  return the_address_map;
}

//------------------------------------------------------------------------------
void Memory_map::read_yaml( void )
{
  YAML::Node root;
  try {
    root = YAML::LoadFile( singleton.m_yaml_filename );
  } catch (const YAML::Exception& e) {
    REPORT( FATAL, e.what() );
  }
  // Read all origin instances
  for( const auto& origin : root ) {
    // Add origin entry to map
    origin_name = origin.first.as<string>();
    m_origin_map[ origin_name ] = Origin_info();
    Origin_info& origin_map{ this->origin_map()[ origin_name ]};
    if( origin.second["type"] )
      origin_map.kind = origin.second["kind"].as<string>();
    if( origin.second["base"] )
      to_int( origin_map.base, origin.second["base"].as<string>() );
    if( origin.second["size"] )
      to_int( origin_map.size, origin.second["size"].as<string>() );
    for( const auto& target : origin.second["targ"] ) {
      // Add target entry to origin
      string targ_name = target.first.as<string>();
      origin_map[ targ_name ] = Target_info();
      Target_info& info{ origin_map[ targ_name ] };
      info.name = targ_name;
      if( target.second["addr"] )
        to_int( info.base, target.second["addr"].as<string>() );
      if( target.second["size"] )
        to_int( info.size, target.second["size"].as<string>() );
      if( target.second["irq"] )
        to_int( info.kind, target.second["irq"].as<string>() );
      if( target.second["kind"] )
        info.kind = target.second["kind"].as<string>();
      if( info.size != UNASSIGNED )
        info.last = info.base + info.size - 1;
    }//endforeach target
  }//endforeach origin
}

//------------------------------------------------------------------------------
Memory_map& Memory_map::instance( void )
{
  static Memory_map the_map;
  return the_map;
}

//------------------------------------------------------------------------------
Origin_map& Memory_map::origin_map( void ) {
  return instance().m_origin_map;
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
