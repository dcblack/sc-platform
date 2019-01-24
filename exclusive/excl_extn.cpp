//FILE: rmx_extn.cpp (systemc)
#include "exclusive/excl_extn.hpp"
#include "report/report.hpp"
namespace {
  const char* MSGID{ "/Doulos/Example/Excl_extn" };
}
using namespace sc_core;
using namespace tlm;

//------------------------------------------------------------------------------
Excl_extn::Excl_extn( void ) //< Constructor
{
}

//------------------------------------------------------------------------------
tlm_extension_base*
Excl_extn::clone( void ) const
{
  return extn;
}
 
//------------------------------------------------------------------------------
void
Excl_extn::copy_from( tlm_extension_base const& extn )
{
}

//------------------------------------------------------------------------------
bool
Excl_extn::exclusive( const tlm::tlm_generic_payload& trans, bool& open )
{
  if( trans.is_read() ) {
    m_success = true;
    open = true;
  }
  else {
    m_success = open;
    open = false;
  }
  return m_success;
}
////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
