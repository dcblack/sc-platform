#ifndef SECURE_EXTN_HPP
#define SECURE_EXTN_HPP

struct Secure_extn
: tlm::tlm_extension<Secure_extn>
{
  Secure_extn( void );
  virtual ~Secure_extn( void ) = default;
  virtual tlm::tlm_extension_base* clone( void ) const override;
  virtual void copy_from(tlm::tlm_extension_base const& extn) override;
  int level { 1 };
};

#endif /*SECURE_EXTN_HPP*/
