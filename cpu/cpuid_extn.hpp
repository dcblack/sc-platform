#ifndef CPUID_EXTN_HPP
#define CPUID_EXTN_HPP

#include <tlm>

struct Cpuid_extn
: tlm::tlm_extension<Cpuid_extn>
{
  Cpuid_extn( sc_core::sc_module* cpu = nullptr );
  virtual tlm_extension_base* clone( void ) const;
  virtual void copy_from( tlm_extension_base const& extn );
  const char* name( void ) const;
  void set_module_ptr( sc_core::sc_module* cpu );
  sc_core::sc_module* get_module_ptr( void ) const;
private:
  sc_core::sc_module* m_cpu_ptr;
};

#endif/*CPUID_EXTN_HPP*/
