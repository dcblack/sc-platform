#ifndef MMU_EXTN_HPP
#define MMU_EXTN_HPP

struct Mmu_extn
: tlm::tlm_extension<Mmu_extn>
{
  // Local types
  enum class Mmu_op { NONE, ENABLE, DISABLE, INVALIDATE_ALL, INVALIDATE_ADDR };
  // Constructors
  Mmu_extn( void );
  virtual ~Mmu_extn( void ) = default;
  virtual tlm::tlm_extension_base* clone( void ) const override;
  virtual void copy_from(tlm::tlm_extension_base const& extn) override;
  void   set_mmu_op( Mmu_op op )  { m_op = op; }
  Mmu_op get_mmu_op( void ) const { return m_op; }
private:
  Mmu_op m_op { Mmu_op::NONE };
};

#endif /*MMU_EXTN_HPP*/
