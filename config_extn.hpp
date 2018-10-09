#ifndef CONFIG_EXTN_H
#define CONFIG_EXTN_H

#include "config.hpp"

struct Config_extn
: tlm::tlm_extension<Config_extn>
{
  Config_extn( void );
  virtual tlm::tlm_extension_base* clone() const override;
  virtual void copy_from(tlm::tlm_extension_base const& extn) override;
  Config config;
};

#endif /*CONFIG_EXTN_H*/
