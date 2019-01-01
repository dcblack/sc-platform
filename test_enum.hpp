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
enum class Test : int {
  TEST_ENUMS(TEST_KEY)
  };
#undef TEST_KEY

std::string to_string( const Test& elt );
bool is_Test( const std::string& str ) noexcept;
Test to_Test( const std::string& str );
std::ostream& operator<<( std::ostream& os, const Test& rhs );
std::istream& operator>>( std::istream& is, Test& rhs );
bool operator<( const Test& lhs, const Test& rhs ) {
  return std::underlying_type<Test>::type(rhs)
         < std::underlying_type<Test>::type(rhs);
}
Test operator++(Test& x) {
  return x = (Test)(std::underlying_type<Test>::type(x) + 1); 
}
Test operator*(Test c) {
  return c;
}
Test begin(Test r) {
  return (Test)std::underlying_type<Test>::type(0);
}
Test end(Test r) {
  return Test::end;
}

#endif /*TEST_ENUM_HPP*/
