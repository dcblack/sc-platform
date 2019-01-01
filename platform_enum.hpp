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

std::string   to_string  ( const Platform& elt    );
bool          is_Platform( const std::string& str ) noexcept;
Platform      to_Platform( const std::string& str );
std::ostream& operator<< ( std::ostream& os, const Platform& rhs );
std::istream& operator>> ( std::istream& is, Platform& rhs );
bool operator<( const Platform& lhs, const Platform& rhs ) {
  return std::underlying_type<Platform>::type(rhs)
         < std::underlying_type<Platform>::type(rhs);
}
Platform operator++(Platform& x) {
  return x = (Platform)(std::underlying_type<Platform>::type(x) + 1); 
}
Platform operator*(Platform c) {
  return c;
}
Platform begin(Platform r) {
  return (Platform)std::underlying_type<Platform>::type(0);
}
Platform end(Platform r) {
  return Platform::end;
}

#endif /*PLATFORM_ENUM_HPP*/
