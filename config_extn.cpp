//FILE: config_extn.cpp (systemc)
#include "config_extn.hpp"
#include "report.hpp"
namespace {
  const char* MSGID{"/Doulos/Example/Config_extn"};
}
using namespace sc_core;
using namespace tlm;

//------------------------------------------------------------------------------
Config_extn::Config_extn( void ) //< Constructor
{
}

//------------------------------------------------------------------------------
tlm_extension_base*
Config_extn::clone() const
{
  Config_extn* extn = new Config_extn();
  extn->config = config;
  return extn;
}
 
//------------------------------------------------------------------------------
void
Config_extn::copy_from( tlm_extension_base const& extn )
{
  auto config_extn = dynamic_cast<const Config_extn*>( &extn );
  sc_assert( config_extn != nullptr );
  config = config_extn->config;
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$