////////////////////////////////////////////////////////////////////////////////
//
//  #    # ####### ### #                                                         
//  #    #    #     #  #                                                         
//  #    #    #     #  #                                                         
//  #    #    #     #  #                                                         
//  #    #    #     #  #                                                         
//  #    #    #     #  #                                                         
//   ####     #    ### #####                                                     
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UTIL_HPP
#define UTIL_HPP

#include <tlm>
#include <ostream>
#include <iomanip>

sc_core::sc_time rand_ps( double mean, double stddev=0.0 );
std::string to_string( tlm::tlm_command cmd );

template<typename T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& vec );

////////////////////////////////////////////////////////////////////////////////
// Implementation
template<typename T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& vec )
{
  static const int threshold{8};
  os << "{ ";
  size_t i = 0;

  for ( auto& v : vec ) {
    if ( i != 0 ) {
      os << ", ";
    }

    if ( i+1 == vec.size() and vec.size() > threshold ) {
      os << " ...";
    }

    if ( i < threshold or i+1 == vec.size() ) {
      os << i++ << ":" << v;
    }

  }

  os << " }";
  return os;
}

#endif /*UTIL_HPP*/
