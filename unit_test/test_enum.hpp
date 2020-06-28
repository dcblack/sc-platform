#ifndef TEST_ENUM_HPP
#define TEST_ENUM_HPP

#if __cplusplus < 201103L
  #error Requires C++11 or better
#endif

#include <cstdint>
#include <string>
#include <iostream>

#define TEST_ENUMS(ENUM)\
  ENUM( DEFAULT )\
  ENUM( MEMORY  )\
  ENUM( TIMER   )\
  ENUM( PIC     )\
  ENUM( GPIO    )\
  ENUM( UART    )\
  ENUM( DMA     )\
  ENUM( TRIVIAL )\
  ENUM(end)

#define TEST_KEY(_a) _a,
enum class PlatformTest : int {
  TEST_ENUMS(TEST_KEY)
  };
#undef TEST_KEY

std::string platformtest_str( const PlatformTest& elt );
bool is_PlatformTest( const std::string& str ) noexcept;
PlatformTest to_PlatformTest( const std::string& str );
std::ostream& operator<<( std::ostream& os, const PlatformTest& rhs );
std::istream& operator>>( std::istream& is, PlatformTest& rhs );
inline PlatformTest operator++(PlatformTest& x) {
  return x = (PlatformTest)(std::underlying_type<PlatformTest>::type(x) + 1); 
}
inline PlatformTest operator*(PlatformTest c) {
  return c;
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC   diagnostic push
#pragma GCC   diagnostic ignored "-Wunused-parameter"
inline PlatformTest begin(PlatformTest r) {
  return (PlatformTest)std::underlying_type<PlatformTest>::type(0);
}
inline PlatformTest end(PlatformTest r) {
  return PlatformTest::end;
}
#pragma clang diagnostic pop
#pragma GCC   diagnostic pop

#endif /*TEST_ENUM_HPP*/
