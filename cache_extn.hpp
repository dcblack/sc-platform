#ifndef CACHE_EXTN_HPP
#define CACHE_EXTN_HPP

struct Cache_extn
: tlm::tlm_extension<Cache_extn>
{
  // Local types
  using Cache_id_t = uint_least8_t;
  enum class Cache_op { NONE, INVALIDATE_ALL, CLEAN_ALL, INVALIDATE_ADDR, CLEAN_ADDR };
  // Constructors
  Cache_extn( void );
  virtual ~Cache_extn( void ) = default;
  virtual tlm::tlm_extension_base* clone( void ) const override;
  virtual void copy_from(tlm::tlm_extension_base const& extn) override;
  // Accessors
  void       reset( void )                 { m_id = 0; m_op = Cache_op::NONE; }
  void       set_cache_id( Cache_id_t id ) { m_id = id;   }
  void       set_cache_op( Cache_op   op ) { m_op = op;   }
  Cache_id_t get_cache_id( void ) const    { return m_id; }
  Cache_op   get_cache_op( void ) const    { return m_op; }
private:
  Cache_id_t      m_id { 0 }; // 0 => none (use level if unique)
  Cache_op m_op { Cache_op::NONE };
};

#endif /*CACHE_EXTN_HPP*/
