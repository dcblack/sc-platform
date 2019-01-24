//FILE: secure_extn.cpp (systemc)
#include "secure/secure_extn.hpp"
#include "report/report.hpp"
namespace {
  char const * const MSGID{ "/Doulos/Example/Secure_extn" };
}
using namespace sc_core;
using namespace tlm;

//------------------------------------------------------------------------------
Secure_extn::Secure_extn( void ) //< Constructor
{
}

//------------------------------------------------------------------------------
tlm_extension_base*
Secure_extn::clone() const
{
  return extn;
}
 
//------------------------------------------------------------------------------
void
Secure_extn::copy_from( tlm_extension_base const& extn )
{
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
