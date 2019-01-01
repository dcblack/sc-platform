#include "cpuid_extn.hpp"
using namespace sc_core;
using namespace tlm;

//------------------------------------------------------------------------------
// Constructor
Cpuid_extn::Cpuid_extn( sc_core::sc_module* cpu_ptr )
: m_cpu_ptr( cpu_ptr )
{
}

//------------------------------------------------------------------------------
tlm_extension_base* Cpuid_extn::clone( void ) const
{
  return new Cpuid_extn( this->m_cpu_ptr );
}

//------------------------------------------------------------------------------
void Cpuid_extn::copy_from( tlm_extension_base const& extn )
{
  auto cpuid_extn = dynamic_cast<const Cpuid_extn*>( &extn );
  sc_assert( cpuid_extn != nullptr );
  if( m_cpu_ptr == nullptr ) {
    m_cpu_ptr = cpuid_extn->m_cpu_ptr;
  }
  //else silently don't copy to allow use in bridges/adaptors.
}

//------------------------------------------------------------------------------
void Cpuid_extn::set_module_ptr( sc_core::sc_module* cpu )
{
  sc_assert( dynamic_cast<sc_core::sc_module*>( cpu ) != nullptr );
  if( m_cpu_ptr == nullptr ) {
    m_cpu_ptr = cpu;
  }
  else {
    SC_REPORT_ERROR( "/Doulos/Example/Cpuid_extn", "May only set once!" );
  }
}

//------------------------------------------------------------------------------
sc_core::sc_module* Cpuid_extn::get_module_ptr( void ) const
{
  return m_cpu_ptr;
}

//------------------------------------------------------------------------------
const char* Cpuid_extn::name( void ) const
{
  return m_cpu_ptr->name();
}

///////////////////////////////////////////////////////////////////////////////
// Copyright {:YEAR:} by {:COMPANY:}. All rights reserved.
//END main.cpp $Id$
