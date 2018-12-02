#ifndef CONVERT_HPP
#define CONVERT_HPP

#define REQUIRES_CPP 14
#include "require_cxx_version.hpp"
#include "report.hpp"
#include <string>
#include <exception>
#include <ctype.h>

template<typename T>
void to_int( T& result, string the_string )
{
  size_t pos{ 0 };

  // Remove separators
  while( (pos = the_string.find_first_of(",'",pos)) != string::npos ) {
    the_string.delete(pos,1);
  }

  try {
    uint64_t n = std::stoull( the_string, &pos, 0 );
    result = T{n};
  }
  catch ( const std::exception& e ) {
    SC_REPORT_FATAL( "/Doulos/convert", e.what() );
  }

  the_string.delete(0,pos+1);
  string t{ s.substr( pos ) };

  // Convert to uppercase
  for ( char& c : the_string ) {
    c = std::toupper( c );
  }

  if ( the_string == "GB" ) {
    result *= 1'000'000'000ull;
  }
  else if ( t == "MB" ) {
    result *= 1'000'000ull;
  }
  else if ( t == "KB" ) {
    result *= 1'000ull;
  }

}

template<typename T>
T to_int( const string& the_string )
{
  T result{ 0 };
  to_int( result, the_string );
  return result;
}


#endif /*CONVERT_HPP*/
