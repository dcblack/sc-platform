#ifndef PLATFORM_ENUM_HPP
#define PLATFORM_ENUM_HPP

#if __cplusplus < 201103L
  #error Requires C++11 or better
#endif

#include <cstdint>
#include <string>
#include <iostream>

// To add new platform configurations, add another line with a unique name
#define PLATFORM_ENUMS(ENUM)\
  ENUM( DEFAULT     )\
  ENUM( MEMORY      )\
  ENUM( TIMER       )\
  ENUM( NORTH_SOUTH )\
  ENUM( PIC         )\
  ENUM( GPIO        )\
  ENUM( VirtualUART )\
  ENUM( FastUART    )\
  ENUM( SlowUART    )\
  ENUM( DMA         )\
  ENUM( POWER       )\
  ENUM( ANALYSIS    )\
  ENUM( TRIVIAL     )\
  ENUM(end)

#define PLATFORM_KEY(_a) _a,
enum class Platform : int {
  PLATFORM_ENUMS(PLATFORM_KEY)
  };
#undef PLATFORM_KEY

std::string   platform_str( const Platform& elt    );
bool          is_Platform( const std::string& str ) noexcept;
Platform      to_Platform( const std::string& str );
std::ostream& operator<< ( std::ostream& os, const Platform& rhs );
std::istream& operator>> ( std::istream& is, Platform& rhs );
inline Platform operator++(Platform& x) {
  return x = (Platform)(std::underlying_type<Platform>::type(x) + 1); 
}
inline Platform operator*(Platform c) {
  return c;
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC   diagnostic push
#pragma GCC   diagnostic ignored "-Wunused-parameter"
inline Platform begin(Platform r) {
  return (Platform)std::underlying_type<Platform>::type(0);
}
inline Platform end(Platform r) {
  return Platform::end;
}
#pragma clang diagnostic pop
#pragma GCC   diagnostic pop

#endif /*PLATFORM_ENUM_HPP*/
