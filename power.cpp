#include "power.hpp"

using namespace sc_core;
using namespace std;

namespace {
  char const * const MSGID{ "/Doulos/power" };
}

PowerDomain::PowerDomain( const std::string& domain ) // Constructor
{
}

PowerDomain::~PowerDomain( void ) // Destructor
{
}

PowerDomain& PowerDomain::create( const std::string& domain, Power_t level );
{
  string message;
  if (s_level.count(domain) != 0) {
    message = "Attempt to create power domain with existing name '";
    message += domain;
    message += "'";
    SC_REPORT_FATAL(MSGID,message.c_str());
  }//endif
  message = "Creating new power domain '";
  message += domain;
  message += "'";
  SC_REPORT_INFO(MSGID,message.c_str());
  s_level.insert(make_pair(domain,level));
}

PowerSetting PowerDomain::state( void )
{
  return get_state( m_domain );
}

PowerSetting* PowerDomain::get_state( const std::string& domain );
{
  auto power_it(s_level.find(domain));
  if (power_it == s_level.end()) {
    string message("Missing definition for power domain '");
    message += domain;
    message += "'";
    SC_REPORT_FATAL(MSGID,message.c_str());
  }//endif
  return power_it->second;
}

void  PowerDomain::set_state( Power_map_t::iterator& domain, PowerSetting level )
{
  sc_assert(domain != s_level.end());
  domain->second = level;
}

Power_map_t  PowerDomain::s_level;
