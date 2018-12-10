//FILE: configuration.cpp (systemc)
#include "configuration.hpp"
#include "report.hpp"
#include <cstdint>
#include <vector>

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

std::unordered_map<size_t, std::unique_ptr<Configuration::Function_t>> Configuration::s_function;

//------------------------------------------------------------------------------
// Constructor
Configuration::Configuration( bool use_defaults )
{
  if ( use_defaults ) {
    set_defaults();
  }
}

//------------------------------------------------------------------------------
// Copy constructor
Configuration::Configuration( const Configuration& rhs )
{
  for( auto& v : rhs.m_data_map ) {
    INFO( MEDIUM, "Assigning m_data_map[" << v.first << "]" );
    size_t v_type_hash = v.second.type().hash_code();
    //< Use stored assign functor
    Configuration::s_function[v_type_hash]
    ->assign( v.first, m_data_map[v.first], v.second);
  }
  for( auto& v : rhs.m_dflt_map ) {
    size_t v_type_hash = v.second.type().hash_code();
    //< Use stored assign functor
    Configuration::s_function[v_type_hash]
    ->assign( v.first, m_dflt_map[v.first], v.second);
  }
  m_reqd_key_set = rhs.m_reqd_key_set;
}

//------------------------------------------------------------------------------
// Copy assignment
Configuration& Configuration::operator=( const Configuration& rhs )
{
  if( this != &rhs ) {
    m_data_map.clear();
    for( auto& v : rhs.m_data_map ) {
      size_t v_type_hash = v.second.type().hash_code();
      //< Use stored assign functor
      Configuration::s_function[v_type_hash]
      ->assign( v.first, m_data_map[v.first], v.second);
    }
    m_dflt_map.clear();
    for( auto& v : rhs.m_dflt_map ) {
      size_t v_type_hash = v.second.type().hash_code();
      //< Use stored assign functor
      Configuration::s_function[v_type_hash]
      ->assign( v.first, m_dflt_map[v.first], v.second);
    }
    m_reqd_key_set = rhs.m_reqd_key_set;
  }
  return *this;
}

//------------------------------------------------------------------------------
void Configuration::set_defaults( void )
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
  config_key( *this, "target_size",  Depth_t( 0 ), true );
  // Base address
  config_key( *this, "target_base",  Addr_t( ~0ULL ), true );
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
bool Configuration::operator==( const Configuration& rhs ) const
{
  if( m_data_map.size() != rhs.m_data_map.size() ) return false;
  for( auto& v : m_data_map ) {
    auto p = rhs.m_data_map.find( v.first );
    // Does the key exist on the rhs?
    if ( p == rhs.m_data_map.end() ) return false;
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
void Configuration::update( const Configuration& rhs )
{
  for( auto v : rhs.m_data_map ) {
    m_data_map[v.first] = v.second;
  }
}

//------------------------------------------------------------------------------
// Remove data for specified field
void Configuration::erase( const string& field )
{
  m_data_map.erase( field );
}

//------------------------------------------------------------------------------
// Remove all data
void Configuration::clear_data( void )
{
  m_data_map.clear();
}

//------------------------------------------------------------------------------
// Add a key
void Configuration::addkey( const string& field, boost::any value, bool reqd )
{
  // Don't add if it already exists
  if ( m_dflt_map.count( field ) != 0 ) {
    // Warn if different type
    if( m_dflt_map[field].type() != value.type() ) {
      REPORT( WARNING, "Field '" << field << "' type mismatch." );
    } else {
      REPORT( WARNING, "Field '" << field << "' already defined "
                    << "- ignoring.\n"
                    << "Use delkey(field) beforehand redefining."
            );
    }
    return;
  }

  // Insert default value
  m_dflt_map[field] = value;

  if ( reqd ) {
    m_reqd_key_set.insert( field );
  }
}

//------------------------------------------------------------------------------
// Remove a key
void Configuration::delkey( const string& field )
{
  m_dflt_map.erase( field );
  m_reqd_key_set.erase( field );
}

//------------------------------------------------------------------------------
// Remove all keys
void Configuration::clear_keys( void )
{
  m_dflt_map.clear();
  m_reqd_key_set.clear();
}

//------------------------------------------------------------------------------
bool Configuration::is_key( const string& field ) const
{
  return m_dflt_map.count( field ) == 1;
}

//------------------------------------------------------------------------------
bool Configuration::has_key( const string& field ) const
{
  return m_data_map.count( field ) == 1;
}

//------------------------------------------------------------------------------
bool Configuration::is_reqd( const string& field ) const
{
  return m_reqd_key_set.count( field ) == 1;
}

//------------------------------------------------------------------------------
bool Configuration::full( void ) const
{
  return m_data_map.size() == m_dflt_map.size();
}

//------------------------------------------------------------------------------
bool Configuration::empty( void ) const
{
  return m_data_map.empty();
}

//------------------------------------------------------------------------------
bool Configuration::has_reqd( void ) const
{
  for( const auto& v : m_reqd_key_set ) {
    if ( m_data_map.count( v ) == 0 ) {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------------
int Configuration::count( void ) const
{
  return m_data_map.size();
}

//------------------------------------------------------------------------------
int Configuration::avail( void ) const
{
  return m_dflt_map.size();
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const Configuration& rhs )
{
  std::vector<string> fields;
  static std::vector<string> special 
  { "name"
  , "kind"
  , "object_ptr"
  , "target_base"
  , "target_size"
  };
  size_t max_width = 0;

  size_t n = rhs.m_data_map.size();
  fields.reserve( rhs.m_data_map.size() );
  for( auto v : rhs.m_data_map ) {
    if( max_width < v.first.length() ) {
      max_width = v.first.length();
    }
    bool is_special{ false };
    for( auto f : special ) {
      is_special |= ( v.first == f );
    }
    if( not is_special ) fields.push_back( v.first );
  }
  std::sort( fields.begin(), fields.end() );

  for( const string& field : special ) {
    if( rhs.m_data_map.count( field ) == 0 ) continue;
    os << "  " << setw(max_width) << (field + ": ");
    boost::any v{ rhs.m_data_map.find( field )->second };
    size_t v_type_hash = v.type().hash_code();
    Configuration::s_function[v_type_hash]->printer( os, v); //< Use stored printing functor
    if( --n ) os << "\n";
  }

  for( const string& field : fields ) {
    os << "  " << setw(max_width) << (field + ": ");
    boost::any v{ rhs.m_data_map.find( field )->second };
    size_t v_type_hash = v.type().hash_code();
    Configuration::s_function[v_type_hash]->printer( os, v); //< Use stored printing functor
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
// This serves both as an example and a simple unit test
#include "configuration.hpp"
#include "report.hpp"
#include "summary.hpp"
#include <iostream>
using namespace std;
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
    m_configuration.set( "name", string( name() ) );
    m_configuration.set( "kind", string( kind() ) );
    m_configuration.set( "object_ptr", uintptr_t( this ) );
  }

  //----------------------------------------------------------------------------
  void test_thread( void ) {
    INFO( MEDIUM, "Initial m_configuration\n" << m_configuration );
    INFO( MEDIUM, "m_configuration has "  << m_configuration.avail() << " available options." );
    INFO( MEDIUM, "m_configuration has "  << m_configuration.count() << " entries." );
    INFO( MEDIUM, "m_configuration is "   << is( !m_configuration.empty() ) << "empty." );
    INFO( MEDIUM, "m_configuration does " << is( m_configuration.has_reqd() ) << "meet requirements." );

    config_key( m_configuration, "debug", false );

    // Set some values
    Configuration t_configuration;
    INFO( MEDIUM, "t_configuration has "  << t_configuration.count() << " entries." );
    INFO( MEDIUM, "t_configuration is "   << is( t_configuration.empty() ) << "empty." );
    t_configuration.set( "target_size",  Depth_t( 4 * KB ) );
    t_configuration.set( "target_base",  Addr_t( 0xC0DE'0000'CAFE ) );
    t_configuration.set( "read_clocks",  uint32_t(2) );
    m_configuration.set( "write_clocks", uint32_t(3) );

    // Test update
    m_configuration.update( t_configuration );
    uint32_t t;
    m_configuration.get( "read_clocks", t );
    INFO( MEDIUM, "read clocks is " << t );
    m_configuration.get( "write_clocks", t );
    INFO( MEDIUM, "write clocks is " << t );

    // Test copy and assign
    Configuration t2( m_configuration );
    INFO( MEDIUM, "Copy constructed." );
    Configuration t3;
    t3 = m_configuration;
    INFO( MEDIUM, "Assigned." );
    INFO( MEDIUM, "t3 contains:\n" << t3 );
    INFO( MEDIUM, "t3 is " << is( t3 == m_configuration ) << "equivalent to m_configuration" );
    sc_stop();
  }

  //----------------------------------------------------------------------------
  void start_of_simulation( void ) override {
    Summary::starting_simulation();
  }

  //----------------------------------------------------------------------------
  void end_of_simulation( void ) override {
    Summary::finished_simulation();
  }

  //----------------------------------------------------------------------------
  const char* kind() const override {
    return "Top_module";
  }
  // Attributes
  Configuration m_configuration;
};

//------------------------------------------------------------------------------
int sc_main( int argc, char* argv[] )
{
  for(int i=1; i<sc_core::sc_argc(); ++i) {
    std::string arg(sc_core::sc_argv()[i]);
    if (arg == "-debug") {
      sc_core::sc_report_handler::set_verbosity_level(SC_DEBUG);
      SC_REPORT_INFO( "/Doulos/Example/config_example", "Verbosity level set to DEBUG" );
    }
  }
  Summary::starting_elaboration();
  Top_module top( "top" );
  sc_start();
  return Summary::report();
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
