#ifndef MACROS_H
#define MACROS_H
#include <stdint.h>

/*
 *******************************************************************************
 *                                                                             *
 *    This probably needs to be rethought and implemented with a clean bit     *
 *    manipulation class or perhaps std::bitset or sc_bv<N>. Perhaps a         *
 *    C++ specific macros.hpp.                                                 *
 *                                                                             *
 *******************************************************************************
 */

#define BIT(n) (1ull<<(n))      /* 0 <= n < 32 */
#define BITS(w)   ((uint32_t)((1ull<<(w))-1)) /* 0 <  w < 32 */
#define MASK(reg) ((uint32_t)(BITS(reg##_BITS)<<reg##_LSB))
#define SET_FIELD(reg,var,val) var=(var&~reg##_MASK)|((val<<reg##_LSB)&reg##_MASK)
#define GET_FIELD(reg,val) ((val&reg##_MASK)>>reg##_LSB)
#define IS_NONZERO(reg,val) ((val&reg##_MASK) != 0ull)

enum GenericFields : uint32_t
{   LE_BYTE0_LSB          =  0
  , LE_BYTE0_BITS         =  8
  , LE_BYTE0_MASK         = MASK( LE_BYTE0 )
,   LE_BYTE1_LSB          =  8
  , LE_BYTE1_BITS         =  8
  , LE_BYTE1_MASK         = MASK( LE_BYTE1 )
,   LE_BYTE2_LSB          =  16
  , LE_BYTE2_BITS         =  8
  , LE_BYTE2_MASK         = MASK( LE_BYTE2 )
,   LE_BYTE3_LSB          =  24
  , LE_BYTE3_BITS         =  8
  , LE_BYTE3_MASK         = MASK( LE_BYTE3 )
,   LE_WORD0_LSB          =  0
  , LE_WORD0_BITS         =  16
  , LE_WORD0_MASK         = MASK( LE_WORD0 )
,   LE_WORD1_LSB          =  16
  , LE_WORD1_BITS         =  16
  , LE_WORD1_MASK         = MASK( LE_WORD1 )
};

#endif /*MACROS_H*/
