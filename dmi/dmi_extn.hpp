#ifndef DMI_EXTN_HPP
#define DMI_EXTN_HPP

#include <tlm>

struct Dmi_extn
: tlm::tlm_extension<Dmi_extn>
{
  Dmi_extn( void );
  virtual ~Dmi_extn( void ) = default;
  virtual tlm::tlm_extension_base* clone( void );
  virtual void copy_from( tlm::tlm_extension_base const& ext );
  virtual void free( void );
};

#endif /*DMI_EXTN_HPP*/
