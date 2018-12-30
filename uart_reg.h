#ifndef USART_REG_H
#define USART_REG_H

// Restricted to C 2011 syntax to allow use by embedded C developers

#include "macros.h"
#include <stdint.h>

// Bit masks
enum PicField : uint32_t
{ USART_{:1st:}_LSB         =  0 , USART_{:1st:}_BITS      =  8 , USART_{:1st:}_MASK      = MASK(PIC_{:1st:})
, USART_{:2nd:}_LSB         =  0 , USART_{:2nd:}_BITS      =  8 , USART_{:2nd:}_MASK      = MASK(PIC_{:2nd:})
};

// Convenience
typedef struct 
{
  uint32_t         {:1st};
  uint32_t         {:2nd};
} Usart_regs_t;

// Address offsets
enum PicAddr : uint64_t
{ USART_{:1st:}_REG    =  0
, USART_{:2nd:}_REG    =  4
, USART_REGS_SIZE   = sizeof(Usart_regs_t)
};

#endif /*USART_REG_H*/
