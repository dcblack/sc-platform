//FILE: dmi_extn.cpp (systemc)
#include "dmi_extn.hpp"
#include "report.hpp"
namespace {
  char const * const MSGID{"/Doulos/Example/Dmi_extn"};
}
using namespace sc_core;
using namespace tlm;

//------------------------------------------------------------------------------
Dmi_extn::Dmi_extn(void) //< Constructor
{
}

//------------------------------------------------------------------------------
tlm_extension_base*
Dmi_extn::clone( void )
{
  NOT_YET_IMPLEMENTED();
  return nullptr;
}

//------------------------------------------------------------------------------
void
Dmi_extn::copy_from(tlm_extension_base const& ext)
{
  NOT_YET_IMPLEMENTED();
}

//------------------------------------------------------------------------------
void
Dmi_extn::free(void)
{
  NOT_YET_IMPLEMENTED();
  delete this;
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
