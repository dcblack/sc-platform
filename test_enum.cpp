#include "test_enum.hpp"
#include <algorithm>
#include <exception>

#define TEST_KEY(_a) #_a,
namespace {
using cstr = const char * const;
cstr s_test_str[] = {
  TEST_ENUMS(TEST_KEY)
};
}
#undef KEY

//------------------------------------------------------------------------------
std::string test_str( const PlatformTest& elt )
{
  return s_test_str[ static_cast<int>( elt ) ];
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const PlatformTest& rhs )
{
  os << to_string( rhs );
  return os;
}

//------------------------------------------------------------------------------
bool is_PlatformTest( const std::string& str ) noexcept
{
  int size = sizeof( s_test_str ) / sizeof( char* );
  cstr* begin = s_test_str;
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
PlatformTest to_PlatformTest( const std::string& str )
{
  static str_exception e{ "PlatformTest out of bounds!" };
  int size = sizeof( s_test_str ) / sizeof( char* );
  cstr* begin = s_test_str;
  cstr* end = begin + size;
  cstr* ptr = find( begin, end, str );
  if( ptr==end ) throw e;
  return static_cast<PlatformTest>( ptr - begin );
}

//------------------------------------------------------------------------------
std::istream& operator>>( std::istream& is, PlatformTest& rhs )
{
  std::string str;
  is >> str;
  rhs = to_PlatformTest( str );
  return is;
}

#ifdef TEST_ENUM_EXAMPLE
#include <sstream>
#include <iomanip>
int main( void )
{
  std::string input;
  std::istringstream is;
  PlatformTest vu, v0, v1, v2{ PlatformTest::MEMORY };
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
  std::cout << "PlatformTests:" << std::endl;
  for( auto p : PlatformTest() ) {
    std::cout << "   " << p << std::endl;
  }
  // User testing
  do {
    std::cout << "Enter an enum name ('q' to quit): " << std::flush;
    std::cin >> input;
    size_t pos;
    if( not is_PlatformTest( input ) ) {
      if( input != "" ) std::cout << "Not a PlatformTest" << std::endl;
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
    std::cout << "Converted result of '" << input << "' is PlatformTest::" << v0 << std::endl;
  } while ( input != "q" );
  return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Copyright 2019 by Doulos. All rights reserved.
//END test_enum.cpp @(#)$Id$
