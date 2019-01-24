#include "common/log2.h"

template<typename T>
constexpr bool isPowerOfTwo( T n )
{
  return n && ( !( n & ( n - 1 ) ) );
}

template<typename T>
auto log2(T v)
{
  return log2_32( v );
}

// Returns the number of bits
namespace {
constexpr int bits( uint64_t v )
{
  int32_t n = 0;
  while( v ) {
    v >>= 1;
    ++n;
  }
  return n-1;
}
}
