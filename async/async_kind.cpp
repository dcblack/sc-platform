#include "async/async_kind.hpp"
#include <algorithm>
#include <exception>

#define ASYNC_KIND_KEY(_a) #_a,
namespace {
using cstr = const char * const;
cstr s_async_kind_str[] = {
  ASYNC_KIND_ENUMS(ASYNC_KIND_KEY)
};
}
#undef KEY

//------------------------------------------------------------------------------
std::string async_kind_str( const Async_kind& elt )
{
  return s_async_kind_str[ static_cast<int>( elt ) ];
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const Async_kind& rhs )
{
  os << async_kind_str( rhs );
  return os;
}

//------------------------------------------------------------------------------
bool is_Async_kind( const std::string& str ) noexcept
{
  int size = sizeof( s_async_kind_str ) / sizeof( char* );
  cstr* begin = s_async_kind_str;
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
Async_kind to_Async_kind( const std::string& str )
{
  static str_exception e{ "Async_kind out of bounds!" };
  int size = sizeof( s_async_kind_str ) / sizeof( char* );
  cstr* begin = s_async_kind_str;
  cstr* end = begin + size;
  cstr* ptr = find( begin, end, str );
  if( ptr==end ) throw e;
  return static_cast<Async_kind>( ptr - begin );
}

//------------------------------------------------------------------------------
std::istream& operator>>( std::istream& is, Async_kind& rhs )
{
  std::string str;
  is >> str;
  rhs = to_Async_kind( str );
  return is;
}

#ifdef ASYNC_KIND_ENUM_EXAMPLE
#include <sstream>
#include <iomanip>
int main( void )
{
  std::string input;
  std::istringstream is;
  Async_kind vu, v0, v1, v2{ Async_kind::packet };
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
  std::cout << "Async_kinds:" << std::endl;
  for( auto elt : Async_kind() ) {
    std::cout << "   " << elt << std::endl;
  }
  // User testing
  do {
    std::cout << "Enter an enum name ('q' to quit): " << std::flush;
    std::cin >> input;
    size_t pos;
    if( not is_Async_kind( input ) ) {
      if( input != "" ) std::cout << "Not a Async_kind" << std::endl;
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
    std::cout << "Converted result of '" << input << "' is Async_kind::" << v0 << std::endl;
  } while ( input != "q" );
  return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Copyright 2019 by Doulos. All rights reserved.
//END async_kind.cpp @(#)$Id$
