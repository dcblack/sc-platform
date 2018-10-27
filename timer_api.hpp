#ifndef TIMER_API_HPP
#define TIMER_API_HPP

#include "common.hpp"

// Address offsets
enum TimerAddr : Addr_t
{ TIMER_STATUS_REG  =  0 //< Current status
, TIMER_CTRLSET_REG =  4 //< Corresponding bits set
, TIMER_CTRLCLR_REG =  8 //< Corresponding bits cleared
, TIMER_TRIG_LO_REG = 12
, TIMER_TRIG_HI_REG = 16
, TIMER_CURR_LO_REG = 20
, TIMER_CURR_HI_REG = 24
, TIMER_PULSE_REG   = 28
, TIMER_SIZE        = 32/*byte addresses*/
, TIMER_GLOBAL_REG  = 32
};

#define BIT(n) (1ul<<(n))      /* 0 <= n < 32 */
#define MASK(w) ((1ul<<(w))-1) /* 0 <  w < 32 */
#define TIMER_IRQ(n) BIT(16+(n))

// Bit masks
enum TimerField : uint32_t
{ TIMER_QTY_LSB        =  0
  , TIMER_QTY_BITS     =  4
  , TIMER_QTY_MASK     = MASK(TIMER_QTY_BITS)<<TIMER_QTY_LSB
, TIMER_SCALE_LSB      =  4
  , TIMER_SCALE_BITS   =  4
  , TIMER_SCALE_MASK   = MASK(TIMER_SCALE_BITS)<<TIMER_SCALE_LSB
, TIMER_IRQ_LSB        = 16
  , TIMER_IRQ_BITS     = 16
  , TIMER_IRQ_MASK     = MASK(TIMER_IRQ_BITS)<<TIMER_IRQ_LSB
, TIMER_RELOAD_MASK    = (1u<<15)
  , TIMER_HALT         = 0u
  , TIMER_RELOAD       = (1u<<15)
, TIMER_STATE_MASK     = (1u<<14)
  , TIMER_ONESHOT      = 0u
  , TIMER_CONTINUOUS   = (1u<<14)
, TIMER_STARTED_MASK   = (1u<<13)
  , TIMER_STOP         = 0u
  , TIMER_START        = (1u<<13)
, TIMER_INTERRUPT_MASK = (1u<<12)
  , TIMER_DISABLE_IRQ  = 0u
  , TIMER_IRQ_ENABLE   = (1u<<12)
, TIMER_PAUSED_MASK    = (1u<<11)
  , TIMER_PAUSED       = 0u
  , TIMER_NORMAL       = (1u<<11)
, TIMER_FLAGS_MASK     = TIMER_RELOAD_MASK
                       | TIMER_STATE_MASK
                       | TIMER_STARTED_MASK
                       | TIMER_INTERRUPT_MASK
                       | TIMER_PAUSED_MASK
};

// Equivalent structure
struct Timer_reg
{
  volatile uint32_t status { 0x0000'0001 }; // Read-status - write to clear any or all IRQ's
  volatile uint32_t ctrlset{ 0x0000'0000 }; // Read/Write (writes to QTY ignored)
  volatile uint32_t ctrlclr{ 0x0000'0000 }; // Read/Write (writes to QTY ignored)
  volatile uint32_t trig_lo{ 0 }; // Writing clears hi
  volatile uint32_t trig_hi{ 0 }; // 
  volatile uint32_t curr_lo{ 0 }; // Writing clears hi
  volatile uint32_t curr_hi{ 0 };
  volatile uint32_t pulse  { 1 };
};

#ifdef CPU_HPP
struct Timer_api
{
  Timer_api(int timer=-1): m_timer((timer<0)?nTimer():timer) { }
  ~Timer_api( void ) = default;
  // Convenience
  int timer( void ) const { return m_timer; }
  void setup( uint32_t timer_count=0 )
  {
    Addr_t timer_addr = TMR_BASE + m_timer*TIMER_SIZE;
    write( timer_addr + TIMER_TRIG_LO_REG, timer_count );
    write( timer_addr + TIMER_CTRLCLR_REG, TIMER_FLAGS );
    write( timer_addr + TIMER_CTRLSET_REG, TIMER_RELOAD | TIMER_CONTINUOUS | TIMER_IRQ_ENABLE);
    m_setup = true;
  }

  void start( void )
  {
    assert( m_setup );
    Addr_t timer_addr = TIMER_BASE + m_timer*TIMER_SIZE;
    write( timer_addr + TIMER_CTRLSET_REG, TIMER_START );
  }

  bool is_running( void ) const
  {
    Addr_t timer_addr = TIMER_BASE + m_timer*TIMER_SIZE;
    uint32_t timer_value { 0 };
    read( m_timer+TIMER_STATUS_REG, timer_value ); //< burst read of two registers
    uint32_t running_mask = TIMER_START | TIMER_NORMAL;
    return (timer_value & running_mask) == running_mask;
  }

  void stop( void )
  {
    Addr_t timer_addr = TIMER_BASE + m_timer*TIMER_SIZE;
    write( timer_addr + TIMER_CTRLCLR_REG, TIMER_START );
  }

  uint64_t value( void ) const
  {
    Addr_t timer_addr = TIMER_BASE + m_timer*TIMER_SIZE;
    uint64_t timer_value { 0 };
    read( m_timer+TIMER_CURR_LO_REG, timer_value ); //< burst read of two registers
    return timer_value;
  }
private:
  Timer_api( const Timer_api& ) = delete;
  const int  m_timer;
  bool m_setup{false};
  // if true, return the next timer else return the number of timers
  static int nTimer( bool next = true ) { static int timer{0}; if( next ) return timer++; else return timer; }
};
#endif

#endif /*TIMER_API_HPP*/
