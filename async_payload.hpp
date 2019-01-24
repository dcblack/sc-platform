#ifndef ASYNC_PAYLOAD_HPP
#define ASYNC_PAYLOAD_HPP

#include "async_kind.hpp"
#include <systemc>
#include <cstdint>
#include <string>
#include <utility>
#include <ostream>
#include <sstream>
#include <iostream>

/*

About this class
----------------

This class adds a wrapper around data to be sent/received from/to
SystemC and an external process. Some of the fields are left to
the imagination/use of the implementer. For example, if attempting
to synchronize time between two systems, some work will need to be
done thinking about how much and what type of time is represented
by the 64-bit m_time member. The intent is that the time be set by
the originator. Similarly, the m_orig and m_dest members represent
"addresses" for the originator and final destination; however, the
designer will need to settle on definitions on how these should be
setup. Possibly an external mapping would need to be imposed. They
have use in debugging as information is examined during run-time.

*/

template<typename Data_t>
struct Async_payload
{
  static constexpr uint64_t ALLONES = ~uint64_t(0);
  // Constructor
  Async_payload
  ( Async_kind kind
  , Data_t&         data
  , uint64_t   dest=ALLONES
  , uint64_t   orig=ALLONES
  , uint64_t   time=ALLONES
  )
  : m_id   ( next() )
  , m_orig ( orig   )
  , m_dest ( dest   )
  , m_time ( time   )
  , m_kind ( kind   )
  , m_data ( data   )
  {
  }
  Async_payload( void )
  : Async_payload( Data_t() )
  {
  }
  // Destructor
  ~Async_payload( void )
  {
  }
  // Moving is OK, but copying is not
  
  Async_payload( Async_payload&& rhs ) nonexcept // Move construct
  {
    *this = std::move( rhs );
  }
  
  Async_payload& operator=( Async_payload&& rhs ) // Move assign
  {
    if ( this != &rhs ) {
      m_id       = rhs.m_id;
      m_orig     = rhs.m_orig;  
      m_dest      = rhs.m_dest;
      m_time     = rhs.m_time;
      m_kind     = rhs.m_kind;
      m_data     = rhs.m_data;
    }
    return *this;
  }

  Async_payload( Async_payload& rhs ) = delete; // Copy Construct
  Async_payload& operator=( Async_payload& rhs ) = delete; // Copy assign

  // Access setters
  void       set_orig ( uint64_t   orig ) { m_orig = orig; }
  void       set_dest ( uint64_t   dest ) { m_dest = dest; }
  void       set_time ( uint64_t   time ) { m_time = time; }
  void       set_kind ( Async_kind kind ) { m_kind = kind; }
  void       set_data ( const Data_t& data ) { m_data = data; }
  void       set_data ( Data_t&&   data ) { m_data = std::move(data); }

  // Access getters
  uint64_t   get_id   ( void ) const { return m_id;   }
  uint64_t   get_orig ( void ) const { return m_orig; }
  uint64_t   get_dest ( void ) const { return m_dest; }
  uint64_t   get_time ( void ) const { return m_time; }
  Async_kind get_kind ( void ) const { return m_kind; }
  Data_t     get_data ( void ) const { return m_data; }
  std::string to_string( void ) const
  {
    static ostringstream ss;
    ss.str("");
    ss << *this;
    return ss.str();
  }

  void pack( std::string& packet ) const
  {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "id"   << YAML::Value << m_id;
    out << YAML::Key << "orig" << YAML::Value << m_orig;
    out << YAML::Key << "dest" << YAML::Value << m_dest;
    out << YAML::Key << "time" << YAML::Value << m_time;
    out << YAML::Key << "kind" << YAML::Value << m_kind;
    size = sizeof( m_data );
    out << YAML::Key << "size" << YAML::Value << size;
    std::vector<uint8_t> u8_vec{ size };
    memcpy( u8_vec.data(), &m_data, size );
    out << YAML::Key << "data" << YAML::Value << u8_vec; 
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
    bool has_errors = false;
    YAML::Node root = YAML::Load( packet );
    for( const auto& elt : root ) {
      auto field = elt.first.as<std::string>();
      size_t size;
      if        ( field == "id"   ) {
        m_id = elt.second.as<uint32_t>();
      } else if ( field == "orig" ) {
        m_orig = elt.second.as<uint64_t>();
      } else if ( field == "dest" ) {
        m_dest = elt.second.as<uint64_t>();
      } else if ( field == "time" ) {
        m_time = elt.second.as<uint64_t>();
      } else if ( field == "kind" ) {
        uint32_t k = elt.second.as<uint32_t>();
        m_kind = static_cast<Async_kind>( k );
      } else if ( field == "size" ) {
        size = elt.second.as<uint32_t>();
      } else if ( field == "data" ) {
        std::vector<uint8_t> u8_vec{ size };
        u8_vec = elt.second.as< decltype( u8_vec ) >();
        memcpy( &m_data, reinterpret_cast<Data_t>( u8_vec.data(), size )
      } else {
        SC_REPORT_ERROR( "/Doulos/Async_payload/unpack", "Unknown fields ignored" );
        has_errors = true;
      }
    }
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
  Data_t      m_data;
};

template<typename Data_t>
std::ostream& operator<<( std::ostream& os, const Async_payload<Data_t>& rhs )
{
  os << pack();
  return os;
}


#endif /*ASYNC_PAYLOAD_HPP*/
