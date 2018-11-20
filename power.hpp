#ifndef POWER_HPP
#define POWER_HPP

#include <systemc>
#include <string>
#include <cstdint>

struct PowerSetting
{
  enum Level : uint8_t
  { off=0
  , gated=1
  , low
  , medium
  , full
  } level { full };
  sc_core::sc_object* root;
};

struct PowerDomain
: sc_core::sc_object
{
  using Power_map_t = std::unordered_map< const char*, PowerSetting* >;
  virtual ~PowerDomain( void ); // Destructor
  static Power_map_t::iterator create( const std::string& domain, Power_t level );
  PowerSetting state( void );
  void reset( void );
  const sc_core::sc_event& event( void ) { return m_event; }
private:
  PowerDomain( const std::string& domain ); // Constructor
  std::vector<sc_core::sc_object*> get_processes( sc_core::sc_object* root );
  static PowerSetting get_state( const std::string& domain );
  static clock_map_t  s_level;
  std::string         m_domain;
  sc_core::sc_event   m_event;
};

#endif /*POWER_HPP*/
