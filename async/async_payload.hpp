#ifndef ASYNC_PAYLOAD_HPP
#define ASYNC_PAYLOAD_HPP

#include "async/async_kind.hpp"
#include "yaml-cpp/yaml.h"
#include <systemc>
#include <cstdint>
#include <string>
#include <utility>
#include <ostream>
#include <sstream>
#include <iomanip>

/*

About this class
----------------

This class adds a wrapper around data to be sent/received from/to
SystemC and an OS external process.

Some of the fields are left to the imagination/use of the implementer. 
For more information, see `ABOUT_ASYNC.md`.

To do
-----
Add callback attributes to pack/unpack data differently. Currently assumes
streaming output will provide a suitable format.

*/

template<typename Data_t>
struct Async_payload
{
  static const uint64_t UNDEFINED = ~uint64_t(0);
  // Constructor
  Async_payload
  ( Async_kind kind
  , const Data_t& data
  , uint64_t      dest=UNDEFINED
  , uint64_t      orig=UNDEFINED
  , uint64_t      time=UNDEFINED
  )
  : m_id   ( next() )
  , m_orig ( orig   )
  , m_dest ( dest   )
  , m_time ( time   )
  , m_kind ( kind   )
  , m_data ( data   )
  , m_has_data( true )
  , m_has_kind( true )
  {
  }
  // Constructor
  Async_payload
  ( Async_kind kind
  , uint32_t   attr
  , uint64_t   dest=UNDEFINED
  , uint64_t   orig=UNDEFINED
  , uint64_t   time=UNDEFINED
  )
  : m_id   ( next() )
  , m_orig ( orig   )
  , m_dest ( dest   )
  , m_time ( time   )
  , m_kind ( kind   )
  , m_attr ( attr   )
  , m_has_data( false )
  , m_has_kind( true )
  {
  }
  void clear( void )
  {
    m_orig = UNDEFINED;
    m_dest = UNDEFINED;
    m_time = UNDEFINED;
    m_attr = UNDEFINED;
    m_has_kind = false;
    m_has_data = false;
  }
  Async_payload( void )
  {
  }
  // Destructor
  ~Async_payload( void )
  {
  }
  // Moving is OK, but copying is not
  
  Async_payload( Async_payload&& rhs ) noexcept // Move construct
  {
    *this = std::move( rhs );
  }
  
  Async_payload& operator=( Async_payload&& rhs ) // Move assign
  {
    if ( this != &rhs ) {
      m_id       = rhs.m_id;
      m_orig     = rhs.m_orig;  
      m_dest     = rhs.m_dest;
      m_time     = rhs.m_time;
      m_kind     = rhs.m_kind;
      m_attr     = rhs.m_attr;
      m_data     = rhs.m_data;
      m_has_data = rhs.m_has_data;
      m_has_kind = rhs.m_has_kind;
    }
    return *this;
  }

  Async_payload( Async_payload& rhs ) = default; // Copy Construct
  Async_payload& operator=( Async_payload& rhs ) = default; // Copy assign

  // Access setters
  void       set_orig ( uint64_t   orig ) { m_orig = orig; }
  void       set_dest ( uint64_t   dest ) { m_dest = dest; }
  void       set_time ( uint64_t   time ) { m_time = time; }
  void       set_kind ( Async_kind kind ) { m_kind = kind; m_has_kind = true; }
  void       set_attr ( uint32_t   attr ) { m_attr = attr; }
  void       set_data ( const Data_t& data ) { m_data = data; m_has_data = true; }
  void       set_data ( Data_t&&   data ) { m_data = std::move(data); m_has_data = true; }

  // Access getters
  uint64_t   get_id   ( void ) const { return m_id;   }
  uint64_t   get_vers ( void ) const { return m_vers; }
  uint64_t   get_orig ( void ) const { return m_orig; }
  uint64_t   get_dest ( void ) const { return m_dest; }
  uint64_t   get_time ( void ) const { return m_time; }
  Async_kind get_kind ( void ) const { return m_kind; }
  uint32_t   get_attr ( void ) const { return m_attr; }
  Data_t     get_data ( void ) const { return m_data; }
  bool       has_kind ( void ) const { return m_has_kind; }
  bool       has_data ( void ) const { return m_has_data; }
  std::string to_string( void ) const
  {
    static std::ostringstream ss;
    ss.str("");
    ss << *this;
    return ss.str();
  }

  void pack( std::string& packet ) const
  {
    // Learn more about YAML emitting at <https://github.com/jbeder/yaml-cpp/wiki/How-To-Emit-YAML>
    YAML::Emitter out;
    out << YAML::Flow;
    out << YAML::BeginMap;
    out << YAML::Key << "id"   << YAML::Value << m_id;
    out << YAML::Key << "vers" << YAML::Value << m_vers;
    out << YAML::Key << "orig" << YAML::Value << m_orig;
    out << YAML::Key << "dest" << YAML::Value << m_dest;
    out << YAML::Key << "time" << YAML::Value << m_time;
    out << YAML::Key << "attr" << YAML::Value << m_attr;
    if ( m_has_kind ) 
      out << YAML::Key << "kind" << YAML::Value << async_kind_str( m_kind );
    if ( m_has_data )
      out << YAML::Key << "data" << YAML::Value << m_data; 
    out << YAML::EndMap;
    packet = std::string( out.c_str() );
  }

  std::string pack( void ) const
  {
    std::string result;
    pack( result );
    return result;
  }

  void unpack( std::string& packet )
  {
    // Learn more about YAML parsing at <https://github.com/jbeder/yaml-cpp/wiki/Tutorial>
    bool has_errors = false;
    YAML::Node root = YAML::Load( packet );
    m_has_data = m_has_kind = false;
    for( const auto& elt : root ) {
      auto field = elt.first.as<std::string>();
      if        ( field == "id"   ) {
        m_id   = elt.second.as<uint32_t>();
      } else if ( field == "vers" ) {
        m_vers = elt.second.as<uint32_t>();
      } else if ( field == "orig" ) {
        m_orig = elt.second.as<uint64_t>();
      } else if ( field == "dest" ) {
        m_dest = elt.second.as<uint64_t>();
      } else if ( field == "time" ) {
        m_time = elt.second.as<uint64_t>();
      } else if ( field == "attr" ) {
        m_attr = elt.second.as<uint32_t>();
      } else if ( field == "kind" ) {
        m_kind = to_Async_kind(elt.second.as<std::string>());
      } else if ( field == "data" ) {
        m_data = elt.second.as<Data_t>();
      } else {
        SC_REPORT_ERROR( "/Doulos/Async_payload/unpack", "Unknown fields ignored" );
        has_errors = true;
      }
    }//endfor
  }


private:
  static uint64_t next( uint64_t incr=1 ) {
    static uint64_t id = 0;
    id += incr;
    return id-incr;
  }
  uint32_t    m_id;
  uint32_t    m_vers{ 0x0001 };
  uint64_t    m_orig;  
  uint64_t    m_dest;     
  uint64_t    m_time;
  Async_kind  m_kind;
  uint32_t    m_attr{ 0 };
  Data_t      m_data;
  bool        m_has_data{ false };
  bool        m_has_kind{ false };
};

template<typename Data_t>
std::ostream& operator<<( std::ostream& os, const Async_payload<Data_t>& rhs )
{
  os << "{"  << std::hex << std::showbase
     << " i:" << rhs.get_id() 
     << " v:" << rhs.get_vers()
     << " a:" << rhs.get_attr()
  ;
  if( rhs.get_orig() == Async_payload<Data_t>::UNDEFINED ) {
    os << " o:UNDEF";
  } else {
    os << " o:" << rhs.get_orig();
  }
  if( rhs.get_dest() == Async_payload<Data_t>::UNDEFINED ) {
    os << " d:UNDEF";
  } else {
    os << " d:" << rhs.get_dest();
  }
  if( rhs.get_time() == Async_payload<Data_t>::UNDEFINED ) {
    os << " t:UNDEF";
  } else {
    os << " t:" << rhs.get_time();
  }
  if( rhs.has_kind() ) {
    os << " k:" << async_kind_str(rhs.get_kind());
  }
  if( rhs.has_data() ) {
    YAML::Emitter ye;
    ye << YAML::Flow << rhs.get_data();
    os << " d: " << ye.c_str();
  }
  os << " }";
  return os;
}

#endif /*ASYNC_PAYLOAD_HPP*/
