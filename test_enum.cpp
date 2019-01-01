#include "test_enum.hpp"
#include <algorithm>
#include <exception>

#define TEST_KEY(_a) #_a,
namespace {
using cstr = const char * const;
cstr test_str[] = {
  TEST_ENUMS(TEST_KEY)
};
}
#undef KEY

//------------------------------------------------------------------------------
std::string to_string( const Test& elt )
{
  return test_str[ static_cast<int>( elt ) ];
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const Test& rhs )
{
  os << to_string( rhs );
  return os;
}

//------------------------------------------------------------------------------
bool is_Test( const std::string& str ) noexcept
{
  int size = sizeof( test_str ) / sizeof( char* );
  cstr* begin = test_str;
  cstr* end = begin + size;
  cstr* ptr = find( begin, end, str );
  return (ptr != end);
}

namespace {
  class str_exception
  : public std::exception
  {
  public:
    str_exception( const char* msg ): m_msg(msg) {}
    const char* what( void ) const noexcept { return m_msg; }
  private:
    const char* m_msg;
  };
}

//------------------------------------------------------------------------------
Test to_Test( const std::string& str )
{
  static str_exception e{ "Test out of bounds!" };
  int size = sizeof( test_str ) / sizeof( char* );
  cstr* begin = test_str;
  cstr* end = begin + size;
  cstr* ptr = find( begin, end, str );
  if( ptr==end ) throw e;
  return static_cast<Test>( ptr - begin );
}

//------------------------------------------------------------------------------
std::istream& operator>>( std::istream& is, Test& rhs )
{
  std::string str;
  is >> str;
  rhs = to_Test( str );
  return is;
}

#ifdef TEST_ENUM_EXAMPLE
#include <sstream>
#include <iomanip>
int main( void )
{
  std::string input;
  std::istringstream is;
  Test vu, v0, v1, v2{ Test::MEMORY };
  std::cout << "v1=" << v1 << std::endl;
  std::cout << "v2=" << v2 << std::endl;
  input = to_string( v2 );
  is.str( input );
  is >> v0;
  is.str("bad entry");
  is.seekg( 0, is.beg );
  try {
    is >> v0;
  }
  catch( std::exception& e ) {
    std::cout << "Successfully caught " << e.what() << std::endl;
  }
  std::cout << "Tests:" << std::endl;
  for( auto p : Test() ) {
    std::cout << "   " << p << std::endl;
  }
  // User testing
  do {
    std::cout << "Enter an enum name ('q' to quit): " << std::flush;
    std::cin >> input;
    size_t pos;
    if( not is_Test( input ) ) {
      if( input != "" ) std::cout << "Not a Test" << std::endl;
      continue;
    }
    is.str( input );
    is.seekg( 0, is.beg );
    v0 = vu;
    try {
      is >> v0;
    }
    catch( std::exception& e ) {
      std::cout << "Caught " << e.what() << std::endl;
    }
    std::cout << "Converted result of '" << input << "' is Test::" << v0 << std::endl;
  } while ( input != "q" );
  return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Copyright 2019 by Doulos. All rights reserved.
//END test_enum.cpp @(#)$Id$
