#include "log2.h"

template<typename T>
auto log2(T v)
{
  return log2_32( v );
}

template<>
auto log2(uint64_t v)
{
  return log2_64( v );
}

constexpr int clog2( uint32_t v )
{
  int32_t n = 0;
  while( v ) {
    v >>= 1;
    ++n;
  }
  return n-1;
}
