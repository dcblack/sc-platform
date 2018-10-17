#ifndef TIMER_REGS_HPP
#define TIMER_REGS_HPP

/* BEGIN_DOCUMENTATION

Timer module may implement 1 to 16 timers. The number implemented may be read
from the QTY field of the status register of any timer.

Each timer contains seven registers.

The timer IRQ mask field of any timer will indicate the interrupt status of
all timers.

*  END_DOCUMENTATION
*/

// Address offsets
enum TimerAddr : Addr_t
{ TIMER_STATUS_REG  =  0
, TIMER_CTRLSET_REG =  4
, TIMER_CTRLCLR_REG =  8
, TIMER_LOAD_LO_REG = 12
, TIMER_LOAD_HI_REG = 16
, TIMER_CURR_LO_REG = 20
, TIMER_CURR_HI_REG = 24
, TIMER_PULSE_REG   = 28
, TIMER_SIZE        = 32/*byte addresses*/
, TIMER_GLOBAL_REG  = 32
};

/*
 Control/status register
 Bit | Meaning
 --: | -------
  31 | IRQ15 interrupt active
  30 | IRQ14 interrupt active
  29 | IRQ13 interrupt active
  28 | IRQ12 interrupt active
  27 | IRQ11 interrupt active
  26 | IRQ10 interrupt active
  25 | IRQ9  interrupt active
  24 | IRQ8  interrupt active
  23 | IRQ7  interrupt active
  22 | IRQ6  interrupt active
  21 | IRQ5  interrupt active
  20 | IRQ4  interrupt active
  19 | IRQ3  interrupt active
  18 | IRQ2  interrupt active
  17 | IRQ1  interrupt active
  16 | IRQ0  interrupt active
  15 | Reload on over/underflow
  14 | One-shot/Continuous 
  13 | Run/Stop
  12 | Enable/Disable Interrupt
  11 | Count Up/Down
  10 | Overflowed 64 bits
   9 | Paused
   8 | Reserved (RAZ)
   7 | Scale[3] - Scale clock
   6 | Scale[2]
   5 | Scale[1]
   4 | Scale[0]
   3 | Timers[3] - Number of timers
   2 | Timers[2]
   1 | Timers[1]
   0 | Timers[0]
*/

// Bit masks
enum TimerField : uint32_t
{ TIMER_QTY_MASK       = 0x0000'000Fu
, TIMER SCALE_MASK  = 0x0000'00F0u
, TIMER_IRQ_MASK    = 0xFFFF'0000u
,   TIMER_IRQ0  = (1u<<16) , TIMER_IRQ1  = (1u<<17) , TIMER_IRQ2  = (1u<<18) , TIMER_IRQ3  = (1u<<19)
,   TIMER_IRQ4  = (1u<<20) , TIMER_IRQ5  = (1u<<21) , TIMER_IRQ6  = (1u<<22) , TIMER_IRQ7  = (1u<<23)
,   TIMER_IRQ8  = (1u<<24) , TIMER_IRQ9  = (1u<<25) , TIMER_IRQ10 = (1u<<26) , TIMER_IRQ11 = (1u<<27)
,   TIMER_IRQ12 = (1u<<28) , TIMER_IRQ13 = (1u<<29) , TIMER_IRQ14 = (1u<<30) , TIMER_IRQ15 = (1u<<31)
, TIMER_RELOAD_MASK    = (1u<<15)     , TIMER_NOLOAD      = 0u           , TIMER_RELOAD     = (1u<<15)
, TIMER_STATE_MASK     = (1u<<14)     , TIMER_ONESHOT     = 0u           , TIMER_CONTINUOUS = (1u<<14)
, TIMER_RUNNING_MASK   = (1u<<13)     , TIMER_STOP        = 0u           , TIMER_START      = (1u<<13)
, TIMER_INTERRUPT_MASK = (1u<<12)     , TIMER_DISABLE_IRQ = 0u           , TIMER_ENABLE_IRQ = (1u<<12)
, TIMER_DIRECTION_MASK = (1u<<11)     , TIMER_COUNT_DOWN  = 0u           , TIMER_COUNT_UP   = (1u<<11)
, TIMER_OVERFLOW_MASK  = (1u<<10)     , TIMER_OVERFLOWED  = (1u<<10)
, TIMER_PAUSED_MASK    = (1u<< 9)     , TIMER_PAUSED      = (1u<< 9)
, TIMER_CLEAR_MASK = 0x0000'FC00
};

// Equivalent structure
struct Timer_reg
{
  volatile uint32_t status { 0x0000'0001 }; // Read-status - write to clear any or all IRQ's
  volatile uint32_t ctrlset{ 0x0000'0000 }; // Read/Write (writes to QTY ignored)
  volatile uint32_t ctrlclr{ 0x0000'0000 }; // Read/Write (writes to QTY ignored)
  volatile uint32_t load_lo{ 0 }; // Writing clears hi
  volatile uint32_t load_hi{ 0 }; // Writing causes transfer into {curr_hi,curr_lo}
  volatile uint32_t curr_lo{ 0 }; // Writing clears hi
  volatile uint32_t curr_hi{ 0 };
  volatile uint32_t pulse  { 1 };
};

// Convenience
void timer_setup( int timer=0, sc_core::sc_time timer_period )
{
  Addr timer_addr = TIMER_BASE + timer*TIMER_SIZE;
  write( timer_addr + TIMER_LOAD_LO_REG, uint32_t( clock_period / timer_period ) );
  write( timer_addr + TIMER_CTRLCLR_REG, TIMER_CLEAR );
  write( timer_addr + TIMER_CTRLSET_REG, TIMER_RELOAD | TIMER_CONTINUOUS | TIMER_ENABLE_IRQ | TIMER_COUNT_UP );
}

void timer_start( int timer=0 )
{
  Addr timer_addr = TIMER_BASE + timer*TIMER_SIZE;
  write( timer_addr + TIMER_CTRLSET_REG, TIMER_START );
}

void timer_stop( int timer=0 )
{
  Addr timer_addr = TIMER_BASE + timer*TIMER_SIZE;
  write( timer_addr + TIMER_CTRLSET_REG, TIMER_STOP );
}

uint64_t timer_value( int timer )
{
  Addr timer_addr = TIMER_BASE + timer*TIMER_SIZE;
  uint64_t timer_value { 0 };
  read( timer+TIMER_CURR_LO_REG, timer_value ); //< burst read of two registers
  return timer_value;
}

#endif /*TIMER_REGS_HPP*/
