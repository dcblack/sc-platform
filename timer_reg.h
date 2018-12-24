#ifndef TIMER_REG_H
#define TIMER_REG_H

#include <stdint.h>
#include "macros.h"

#define TIMER_IRQ(n) BIT(16+(n))

// Bit masks
enum TimerField : uint32_t
{ TIMER_QTY_LSB        =  0
  , TIMER_QTY_BITS     =  4
  , TIMER_QTY_MASK     = MASK(TIMER_QTY)
, TIMER_SCALE_LSB      =  4
  , TIMER_SCALE_BITS   =  4
  , TIMER_SCALE_MASK   = MASK(TIMER_SCALE)
, TIMER_IRQ_LSB        = 16
  , TIMER_IRQ_BITS     = 16
  , TIMER_IRQ_MASK     = MASK(TIMER_IRQ)
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
typedef struct
{
  volatile uint32_t status;   // Read-status - write to clear any or all IRQ's
  volatile uint32_t ctrlset;  // Read/Write (writes to QTY ignored)
  volatile uint32_t ctrlclr;  // Read/Write (writes to QTY ignored)
  volatile uint32_t load_lo;  // Writing clears hi
  volatile uint32_t load_hi;  // 
  volatile uint32_t curr_lo;  // Writing clears hi
  volatile uint32_t curr_hi;
  volatile uint32_t pulse;
} Timer_reg_t;

// Address offsets
enum TimerAddr : uint64_t
{ TIMER_STATUS_REG  =  0 //< Current status
, TIMER_CTRLSET_REG =  4 //< Corresponding bits set
, TIMER_CTRLCLR_REG =  8 //< Corresponding bits cleared
, TIMER_LOAD_LO_REG = 12
, TIMER_LOAD_HI_REG = 16
, TIMER_CURR_LO_REG = 20
, TIMER_CURR_HI_REG = 24
, TIMER_PULSE_REG   = 28
, TIMER_REGS_SIZE   = sizeof(Timer_reg_t)
, TIMER_GLOBAL_REG  = 32
};

#endif /*TIMER_REG_H*/
