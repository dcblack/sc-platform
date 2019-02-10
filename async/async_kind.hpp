#ifndef ASYNC_KIND_HPP
#define ASYNC_KIND_HPP

#if __cplusplus < 201103L
  #error Requires C++11 or better
#endif

#include <cstdint>
#include <string>
#include <iostream>

#define ASYNC_KIND_ENUMS(ENUM)\
  ENUM( command  )\
  ENUM( stream   )\
  ENUM( parallel )\
  ENUM( packet   )\
  ENUM( graphic  )\
  ENUM( audio    )\
  ENUM( debug    )\
  ENUM( shutdown )\
  ENUM( end      ) //< required

#define ASYNC_KIND_KEY(_a) _a,
enum class Async_kind : int {
  ASYNC_KIND_ENUMS(ASYNC_KIND_KEY)
};
#undef ASYNC_KIND_KEY

std::string async_kind_str( const Async_kind& elt );
bool is_Async_kind( const std::string& str ) noexcept;
Async_kind to_Async_kind( const std::string& str );
std::ostream& operator<<( std::ostream& os, const Async_kind& rhs );
std::istream& operator>>( std::istream& is, Async_kind& rhs );
inline Async_kind operator++(Async_kind& x) {
  return x = (Async_kind)(std::underlying_type<Async_kind>::type(x) + 1); 
}
inline Async_kind operator*(Async_kind c) {
  return c;
}
inline Async_kind begin(Async_kind r) {
  return (Async_kind)std::underlying_type<Async_kind>::type(0);
}
inline Async_kind end(Async_kind r) {
  return Async_kind::end;
}

#endif /*ASYNC_KIND_HPP*/
