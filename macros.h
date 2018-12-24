#ifndef MACROS_H
#define MACROS_H

#define BIT(n) (1ul<<(n))      /* 0 <= n < 32 */
#define BITS(w) ((1ul<<(w))-1) /* 0 <  w < 32 */
#define MASK(reg) (BITS(reg##_BITS)<<reg##_LSB)

#endif /*MACROS_H*/
