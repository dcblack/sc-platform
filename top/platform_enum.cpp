#include "top/platform_enum.hpp"
#include <algorithm>
#include <exception>

#define PLATFORM_KEY(_a) #_a,
namespace {
using cstr = const char * const;
cstr s_platform_str[] = {
  PLATFORM_ENUMS(PLATFORM_KEY)
};
}
#undef KEY

//------------------------------------------------------------------------------
std::string platform_str( const Platform& elt )
{
  return s_platform_str[ static_cast<int>( elt ) ];
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const Platform& rhs )
{
  os << platform_str( rhs );
  return os;
}

//------------------------------------------------------------------------------
bool is_Platform( const std::string& str ) noexcept
{
  int size = sizeof( s_platform_str ) / sizeof( char* );
  cstr* begin = s_platform_str;
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
Platform to_Platform( const std::string& str )
{
  static str_exception e{ "Platform out of bounds!" };
  int size = sizeof( s_platform_str ) / sizeof( char* );
  cstr* begin = s_platform_str;
  cstr* end = begin + size;
  cstr* ptr = find( begin, end, str );
  if( ptr==end ) throw e;
  return static_cast<Platform>( ptr - begin );
}

//------------------------------------------------------------------------------
std::istream& operator>>( std::istream& is, Platform& rhs )
{
  std::string str;
  is >> str;
  rhs = to_Platform( str );
  return is;
}

#ifdef PLATFORM_ENUM_EXAMPLE
#include <sstream>
#include <iomanip>
int main( void )
{
  std::string input;
  std::istringstream is;
  Platform vu, v0, v1, v2{ Platform::TIMER };
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
  std::cout << "Platforms:" << std::endl;
  for( auto p : Platform() ) {
    std::cout << "   " << p << std::endl;
  }
  // User testing
  do {
    std::cout << "Enter an enum name ('q' to quit): " << std::flush;
    std::cin >> input;
    size_t pos;
    if( not is_Platform( input ) ) {
      if( input != "" ) std::cout << "Not a Platform" << std::endl;
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
    std::cout << "Converted result of '" << input << "' is Platform::" << v0 << std::endl;
  } while ( input != "q" );
  return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Copyright 2019 by Doulos. All rights reserved.
//END platform_enum.cpp @(#)$Id$
