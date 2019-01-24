#ifndef CONFIG_EXTN_HPP
#define CONFIG_EXTN_HPP

#include "config/configuration.hpp"

struct Config_extn
: tlm::tlm_extension<Config_extn>
{
  Config_extn( void );
  virtual ~Config_extn( void ) = default;
  virtual tlm::tlm_extension_base* clone( void ) const override;
  virtual void copy_from(tlm::tlm_extension_base const& extn) override;
  Configuration configuration;
};

#endif /*CONFIG_EXTN_HPP*/
