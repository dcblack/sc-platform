#ifndef EXCL_EXTN_HPP
#define EXCL_EXTN_HPP

#include <tlm>

struct Excl_extn
: tlm::tlm_extension<Excl_extn>
{
  Excl_extn( void );
  virtual ~Excl_extn( void ) = default;
  virtual tlm::tlm_extension_base* clone( void ) const override;
  virtual void copy_from(tlm::tlm_extension_base const& extn) override;
  bool exclusive( const tlm::tlm_generic_payload& trans, bool& open );
  void succeeded( void ) const { return m_success; }
private:
  bool m_success { false };
};

#endif /*EXCL_EXTN_HPP*/
