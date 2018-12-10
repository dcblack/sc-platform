#ifndef CONVERT_HPP
#define CONVERT_HPP

#define REQUIRES_CPP 14
#include "require_cxx_version.hpp"
#include "report.hpp"
#include <string>
#include <exception>
#include <ctype.h>

template<typename T>
void to_int( T& result, std::string the_string )
{
  size_t pos{ 0 };

  // Remove separators
  while( (pos = the_string.find_first_of(",'",pos)) != std::string::npos ) {
    the_string.erase(pos,1);
  }

  try {
    int64_t n = std::stoll( the_string, &pos, 0 );
    result = static_cast<T>(n);
  }
  catch ( const std::exception& e ) {
    SC_REPORT_ERROR( "/Doulos/convert", e.what() );
  }

  // Remove the numeric portion
  the_string.erase(0,pos+1);

  // Convert to uppercase
  for ( char& c : the_string ) {
    c = std::toupper( c );
  }

  if ( the_string == "GB" ) {
    result *= 1'000'000'000ull;
  }
  else if( the_string == "MB" ) {
    result *= 1'000'000ull;
  }
  else if( the_string == "KB" ) {
    result *= 1'000ull;
  }
  else if( the_string.size() > 0 and the_string != "B" ) {
    SC_REPORT_ERROR( "/Doulos/convert", (std::string("Unknown suffix '")+the_string+"' on number.").c_str() );
  }

}

template<typename T>
T to_int( const std::string& the_string )
{
  T result{ 0 };
  to_int( result, the_string );
  return result;
}


#endif /*CONVERT_HPP*/
