#ifndef MACROS_H
#define MACROS_H

#define BIT(n) (1ul<<(n))      /* 0 <= n < 32 */
#define BITS(w) ((1ul<<(w))-1) /* 0 <  w < 32 */
#define MASK(reg) (BITS(reg##_BITS)<<reg##_LSB)
#define SET_FIELD(reg,var,val) var=(var&~reg##_MASK)|((val<<reg##_LSB)&reg##_MASK)
#define GET_FIELD(reg,val) ((var&reg##_MASK)>>reg##_LSB)
#define IS_NONZERO(reg,val) ((var&reg##_MASK) != 0)

#endif /*MACROS_H*/
