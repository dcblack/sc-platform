#ifndef TIMER_API_HPP
#define TIMER_API_HPP

#include "cpu.hpp"
#include "memory_map.hpp"
#include "timer_reg.h"

struct Timer_api
{
  // Constructor
  Timer_api(Cpu_module& cpu, Addr_t base, int timer=-1 )
  : m_cpu( cpu )
  , m_base( base )
  , m_timer( (timer<0)?nTimer():timer )
  {
    INFO( MEDIUM, "Referencing timer " << m_timer );
  }

  ~Timer_api( void ) = default;
  // Convenience
  int timer( void ) const { return m_timer; }
  void setup( uint32_t timer_count=0, uint32_t scale=1 )
  {
    Addr_t timer_addr = m_base + m_timer*TIMER_SIZE;
    m_cpu.write32( timer_addr + TIMER_LOAD_LO_REG, timer_count );
    m_cpu.write32( timer_addr + TIMER_CTRLCLR_REG, TIMER_FLAGS_MASK | TIMER_SCALE_MASK );
    m_cpu.write32( timer_addr + TIMER_CTRLSET_REG, TIMER_RELOAD | TIMER_CONTINUOUS | TIMER_IRQ_ENABLE
                                                   | ((scale<<TIMER_SCALE_LSB) & TIMER_SCALE_MASK ) );
    m_setup = true;
  }

  void start( void )
  {
    assert( m_setup );
    Addr_t timer_addr = m_base + m_timer*TIMER_SIZE;
    m_cpu.write32( timer_addr + TIMER_CTRLSET_REG, TIMER_START );
  }

  bool is_running( void ) const
  {
    Addr_t timer_addr = m_base + m_timer*TIMER_SIZE;
    uint32_t timer_value { 0 };
    m_cpu.read32( timer_addr + TIMER_STATUS_REG, timer_value ); //< burst read of two registers
    uint32_t running_mask = TIMER_START | TIMER_NORMAL;
    return (timer_value & running_mask) == running_mask;
  }

  void stop( void )
  {
    Addr_t timer_addr = m_base + m_timer*TIMER_SIZE;
    m_cpu.write32( timer_addr + TIMER_CTRLCLR_REG, TIMER_START );
  }

  uint64_t status( void ) const
  {
    Addr_t timer_addr = m_base + m_timer*TIMER_SIZE;
    uint32_t timer_status { 0 };
    m_cpu.read32( timer_addr + TIMER_STATUS_REG, timer_status );
    return timer_status;
  }

  uint64_t value( void ) const
  {
    Addr_t timer_addr = m_base + m_timer*TIMER_SIZE;
    uint64_t timer_value { 0 };
    m_cpu.read64( timer_addr + TIMER_CURR_LO_REG, timer_value ); //< burst read of two registers
    return timer_value;
  }
private:
  Timer_api( const Timer_api& ) = delete;

  // Helpers
  //------------------------------------------------------------------------------
  // if true, return the next timer else return the number of timers
  static int nTimer( bool next = true )
  {
    static int next_timer { 0 };
    int timer = next_timer++;
    if( next ) return timer;
    else       return next_timer;
  }

  // Attributes
  Cpu_module&  m_cpu;
  const Addr_t m_base;
  const int    m_timer;
  bool         m_setup{false};

  const char * const MSGID{ "/Doulos/Example/Timer_api" };
};

#endif /*TIMER_API_HPP*/
