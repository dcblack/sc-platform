//FILE: config.cpp (systemc)
#include "config.hpp"
#include "report.hpp"
#include <cstdint>
#include <vector>

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

std::unordered_map<size_t, std::unique_ptr<Config::Function_t>> Config::s_function;

//------------------------------------------------------------------------------
// Constructor
Config::Config( bool use_defaults )
{
  if ( use_defaults ) {
    set_defaults();
  }
}

//------------------------------------------------------------------------------
// Copy constructor
Config::Config( const Config& rhs )
{
  for( auto& v : rhs.m_data ) {
    INFO( MEDIUM, "Assigning m_data[" << v.first << "]" );
    size_t v_type_hash = v.second.type().hash_code();
    //< Use stored assign functor
    Config::s_function[v_type_hash]
    ->assign( v.first, m_data[v.first], v.second);
  }
  for( auto& v : rhs.m_dflt ) {
    size_t v_type_hash = v.second.type().hash_code();
    //< Use stored assign functor
    Config::s_function[v_type_hash]
    ->assign( v.first, m_dflt[v.first], v.second);
  }
  m_reqd = rhs.m_reqd;
}

//------------------------------------------------------------------------------
// Copy assignment
Config& Config::operator=( const Config& rhs )
{
  if( this != &rhs ) {
    m_data.clear();
    for( auto& v : rhs.m_data ) {
      size_t v_type_hash = v.second.type().hash_code();
      //< Use stored assign functor
      Config::s_function[v_type_hash]
      ->assign( v.first, m_data[v.first], v.second);
    }
    m_dflt.clear();
    for( auto& v : rhs.m_dflt ) {
      size_t v_type_hash = v.second.type().hash_code();
      //< Use stored assign functor
      Config::s_function[v_type_hash]
      ->assign( v.first, m_dflt[v.first], v.second);
    }
    m_reqd = rhs.m_reqd;
  }
  return *this;
}

//------------------------------------------------------------------------------
void Config::set_defaults( void )
{
  // Remove all old
  clear_keys();
  // Module class (e.g. "Top_module")
  config_key( *this, "kind",          string( "" ), true );
  // Instance path name (i.e. this->name())
  config_key( *this, "name",          string( "" ), true );
  // Object pointer
  config_key( *this, "object_ptr",    uintptr_t(nullptr), true );
  // Desired coding style (AT or LT)
  config_key( *this, "coding_style",  Style::UNKNOWN );
  // Number of addresses responded to
  config_key( *this, "target_depth",  Depth_t( 0 ), true );
  // Base address
  config_key( *this, "target_start",  Addr_t( ~0ULL ), true );
  // If DMI is supported, this optionally enables it
  config_key( *this, "dmi_allowed",   Feature::none );
  // Read/write access type
  config_key( *this, "access",        Access::none );
  // If byte enables are allowed, this enables it
  config_key( *this, "byte_enables",  Feature::none );
  // Required minimum alignment for internal accesses
  config_key( *this, "alignment",     size_t( 1 ) );
  // Maximum number of bytes per burst (0 => infinite for LT)
  config_key( *this, "max_burst",     size_t( 0 ) );
  // Latency of data read
  config_key( *this, "read_clocks",   uint32_t(0) );
  // Latency of data write
  config_key( *this, "write_clocks",  uint32_t(0) );
  // Latency of address setup
  config_key( *this, "addr_clocks",   uint32_t(0) );
  // Clock time used for latency calculations
  config_key( *this, "clock_period",  SC_ZERO_TIME );
}

//------------------------------------------------------------------------------
bool Config::operator==( const Config& rhs ) const
{
  if( m_data.size() != rhs.m_data.size() ) return false;
  for( auto& v : m_data ) {
    auto p = rhs.m_data.find( v.first );
    // Does the key exist on the rhs?
    if ( p == rhs.m_data.end() ) return false;
    // Are they the same type?
    size_t v_type_hash = v.second.type().hash_code();
    size_t p_type_hash = p->second.type().hash_code();
    if ( v_type_hash != p_type_hash ) return false;
    // Do they have the same value? (Uses stored comparison functor.)
    bool equal = s_function[v_type_hash]->equal( v.second, p->second);
    if ( not equal ) return false;
  }
  return true;
}

//------------------------------------------------------------------------------
// Copy all data present from argument
void Config::update( const Config& rhs )
{
  for( auto v : rhs.m_data ) {
    m_data[v.first] = v.second;
  }
}

//------------------------------------------------------------------------------
// Remove data for specified field
void Config::erase( const string& field )
{
  m_data.erase( field );
}

//------------------------------------------------------------------------------
// Remove all data
void Config::clear_data( void )
{
  m_data.clear();
}

//------------------------------------------------------------------------------
// Remove a key
void Config::delkey( const string& field )
{
  m_dflt.erase( field );
  m_reqd.erase( field );
}

//------------------------------------------------------------------------------
// Remove all keys
void Config::clear_keys( void )
{
  m_dflt.clear();
  m_reqd.clear();
}

//------------------------------------------------------------------------------
bool Config::is_key( const string& field ) const
{
  return m_dflt.count( field ) == 1;
}

//------------------------------------------------------------------------------
bool Config::has_key( const string& field ) const
{
  return m_data.count( field ) == 1;
}

//------------------------------------------------------------------------------
bool Config::is_reqd( const string& field ) const
{
  return m_reqd.count( field ) == 1;
}

//------------------------------------------------------------------------------
bool Config::full( void ) const
{
  return m_data.size() == m_dflt.size();
}

//------------------------------------------------------------------------------
bool Config::empty( void ) const
{
  return m_data.empty();
}

//------------------------------------------------------------------------------
bool Config::has_reqd( void ) const
{
  for( const auto& v : m_reqd ) {
    if ( m_data.count( v ) == 0 ) {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------------
int Config::count( void ) const
{
  return m_data.size();
}

//------------------------------------------------------------------------------
int Config::avail( void ) const
{
  return m_dflt.size();
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const Config& rhs )
{
  std::vector<string> fields;
  std::vector<string> special = {"name", "kind", "object_ptr", "target_start", "target_depth"};

  size_t n = rhs.m_data.size();
  fields.reserve( rhs.m_data.size() );
  for( auto v : rhs.m_data ) {
    bool is_special{ false };
    for( auto f : special ) {
      is_special |= ( v.first == f );
    }
    if( not is_special ) fields.push_back( v.first );
  }
  std::sort( fields.begin(), fields.end() );

  for( const string& field : special ) {
    if( rhs.m_data.count( field ) == 0 ) continue;
    os << "  " << field << ": ";
    boost::any v{ rhs.m_data.find( field )->second };
    size_t v_type_hash = v.type().hash_code();
    Config::s_function[v_type_hash]->printer( os, v); //< Use stored printing functor
    if( --n ) os << "\n";
  }

  for( const string& field : fields ) {
    os << "  " << field << ": ";
    boost::any v{ rhs.m_data.find( field )->second };
    size_t v_type_hash = v.type().hash_code();
    Config::s_function[v_type_hash]->printer( os, v); //< Use stored printing functor
    if( --n ) os << "\n";
  }

  return os;
}

////////////////////////////////////////////////////////////////////////////////
//
//  ##### #     #    #    #     # #####  #     #####                             
//  #      #   #    # #   ##   ## #    # #     #                                 
//  #       # #    #   #  # # # # #    # #     #                                 
//  #####    #    #     # #  #  # #####  #     #####                             
//  #       # #   ####### #     # #      #     #                                 
//  #      #   #  #     # #     # #      #     #                                 
//  ##### #     # #     # #     # #      ##### #####                             
//
////////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_EXAMPLE
#include "config.hpp"
#include "report.hpp"
#include <iostream>
using namespace std;
std::ostringstream mout;
// Helper
//------------------------------------------------------------------------------
namespace {
string is( bool flag )
{
  return flag ? "" : "not ";
}
}
//------------------------------------------------------------------------------
SC_MODULE( Top_module )
{
  const char* MSGID{ "/Doulos/Example/Config_example" };

  //----------------------------------------------------------------------------
  SC_CTOR( Top_module ) {
    SC_THREAD( test_thread );
    m_config.set( "name", string( name() ) );
    m_config.set( "kind", string( kind() ) );
    m_config.set( "object_ptr", uintptr_t( this ) );
  }

  //----------------------------------------------------------------------------
  void test_thread( void ) {
    INFO( MEDIUM, "Initial m_config\n" << m_config );
    INFO( MEDIUM, "m_config has "  << m_config.avail() << " available options." );
    INFO( MEDIUM, "m_config has "  << m_config.count() << " entries." );
    INFO( MEDIUM, "m_config is "   << is( !m_config.empty() ) << "empty." );
    INFO( MEDIUM, "m_config does " << is( m_config.has_reqd() ) << "meet requirements." );

    config_key( m_config, "debug", false );

    // Set some values
    Config t_config;
    INFO( MEDIUM, "t_config has "  << t_config.count() << " entries." );
    INFO( MEDIUM, "t_config is "   << is( t_config.empty() ) << "empty." );
    t_config.set( "target_depth",  Depth_t( 4 * KB ) );
    t_config.set( "target_start",  Addr_t( 0xC0DE'0000'CAFE ) );
    t_config.set( "read_clocks",  uint32_t(2) );
    m_config.set( "write_clocks", uint32_t(3) );

    // Test update
    m_config.update( t_config );
    uint32_t t;
    m_config.get( "read_clocks", t );
    INFO( MEDIUM, "read clocks is " << t );
    m_config.get( "write_clocks", t );
    INFO( MEDIUM, "write clocks is " << t );

    // Test copy and assign
    Config t2( m_config );
    INFO( MEDIUM, "Copy constructed." );
    Config t3;
    t3 = m_config;
    INFO( MEDIUM, "Assigned." );
    INFO( MEDIUM, "t3 contains:\n" << t3 );
    INFO( MEDIUM, "t3 is " << is( t3 == m_config ) << "equivalent to m_config" );
    sc_stop();
  }

  //----------------------------------------------------------------------------
  const char* kind() const override {
    return "Top_module";
  }
  // Attributes
  Config m_config;
};

//------------------------------------------------------------------------------
int sc_main( int argc, char* argv[] )
{
  for(int i=1; i<sc_core::sc_argc(); ++i) {
    std::string arg(sc_core::sc_argv()[i]);
    if (arg == "-debug") {
      sc_core::sc_report_handler::set_verbosity_level(SC_DEBUG);
      SC_REPORT_INFO( "/Doulos/example/config", "Verbosity level set to DEBUG" );
    }
  }
  Top_module top( "top" );
  sc_start();
  return 0;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
