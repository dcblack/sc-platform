#ifndef PIC_API_HPP
#define PIC_API_HPP

#include "cpu.hpp"
#include "memory_map.hpp"
#include "pic_reg.h"

struct Pic_api
{
  // Constructor
  Pic_api(Cpu_module& cpu, Addr_t base )
  : m_cpu( cpu )
  , m_base( base )
  {
  }

  ~Pic_api( void ) = default;

  // Convenience
  uint32_t get_ident( void );
  void     setup( void );
  uint16_t get_source_count( void );
  uint16_t get_target_count( void );
  uint32_t get_targetid( void );
  uint32_t get_target( void );
  void     set_target( void );
  void     set_mask( uint8_t level );
  uint8_t  get_mask( void );
  void     enable_interface( void ):
  void     disable_interface( void );
  void     clearall( void );
  uint32_t get_source( int intid );
  void     set_source( int intid, uint32_t source );
  void     set_pending( int intid, bool pending );
  bool     get_pending( int intid );
  void     set_enabled( int intid, bool enabled );
  bool     get_enabled( int intid );
  void     set_priority( int intid, uint8_t level );
  uint8_t  get_priority( int intid );
  int      get_next( void );
  void     set_done( int intid = -1 );
  void     set_targets( int intid, uint32_t targets );

  void setup( void )
  {
    m_setup = true;
  }

  void enable_interface( void )
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.read32 ( pic_addr + PIC_TARGET_REG, result );
    result |= PIC_ENABLED_MASK;
    m_cpu.write32( pic_addr + PIC_TARGET_REG, result );
  }

  void     disable_interface( void )
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.read32 ( pic_addr + PIC_TARGET_REG, result );
    result &= ~PIC_ENABLED_MASK;
    m_cpu.write32( pic_addr + PIC_TARGET_REG, result );
  }

  uint32_t get_source( int intid )
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.write32( pic_addr + PIC_SELECT_REG, intid );
    m_cpu.read32 ( pic_addr + PIC_SOURCE_REG, result );
    return result;
  }

  void set_source( int intid, uint32_t source );
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + PIC_SELECT_REG, intid );
    m_cpu.write32( pic_addr + PIC_SOURCE_REG, source );
  }

  bool     get_pending( int intid );
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.write32( pic_addr + PIC_SELECT_REG, intid );
    m_cpu.read32 ( pic_addr + PIC_SOURCE_REG, result );
    return (result & PIC_PENDING_MASK) != 0;
  }

  void     set_pending( int intid, bool pending );
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.write32( pic_addr + PIC_SELECT_REG, intid );
    m_cpu.read32 ( pic_addr + PIC_SOURCE_REG, result );
    result &= ~PIC_PENDING_MASK;
    result |= (pending?PIC_PENDING_MASK:0);
    m_cpu.write32( pic_addr + PIC_SOURCE_REG, result );
  }

  bool     get_enabled( int intid );
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.write32( pic_addr + PIC_SELECT_REG, intid );
    m_cpu.read32 ( pic_addr + PIC_SOURCE_REG, result );
    return (result & PIC_ENABLED_MASK) != 0;
  }

  void     set_enabled( int intid, bool enabled );
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.write32( pic_addr + PIC_SELECT_REG, intid );
    m_cpu.read32 ( pic_addr + PIC_SOURCE_REG, result );
    result &= ~PIC_ENABLED_MASK;
    result |= (pending?PIC_ENABLED_MASK:0);
    m_cpu.write32( pic_addr + PIC_SOURCE_REG, result );
  }

  void     set_mask( uint8_t level )
  {
    assert( m_setup );
    uint32_t result;
    m_cpu.read32 ( pic_addr + PIC_TARGET_REG, result );
    result &= ~PIC_PRI_MASK;
    result |= (level & PIC_PRI_MASK) << PIC_PRI_LSB;
    m_cpu.write32( pic_addr + PIC_TARGET_REG, result );
  }

  uint8_t  get_mask( void )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

  int      get_next( void )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

  void     set_done( int intid = -1 )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

  void     set_targets( int intid, uint32_t targets )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

  uint32_t get_ident( void )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

  uint16_t get_source_count( void )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

  uint16_t get_target_count( void )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

  void     clearall( void )
  {
    assert( m_setup );
    m_cpu.write32( pic_addr + {:REG:}, {:VAR:} );
  }

private:
  Pic_api( const Pic_api& ) = delete;
  Pic_api( Pic_api&& ) = delete;
  Pic_api& operator=( const Pic_api& ) = delete;
  Pic_api& operator=( Pic_api&& ) = delete;

  // Attributes
  Cpu_module&  m_cpu;
  const Addr_t m_base;
  bool         m_setup{false};
  int          m_intid{-1};

  const char * const MSGID{ "/Doulos/Example/Pic_api" };
};

#endif /*PIC_API_HPP*/
