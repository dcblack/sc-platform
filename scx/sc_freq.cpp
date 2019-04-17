#include "scx/sc_freq.hpp"

namespace sc_core {

double sc_freq::magnitude( sc_freq_units units ) {
  static double pow[] = { 1.0, 1.0e3, 1.0e6, 1.0e9, 1.0e12 };
  sc_assert( 0 <= units && units < sizeof( pow )/sizeof( pow[0] ) );
  return pow[ units ];
}
std::string sc_freq::str( sc_freq_units units ) {
  switch( units ) {
    case  SC_HZ: return  "Hz";
    case SC_KHZ: return "KHz";
    case SC_MHZ: return "MHz";
    case SC_GHZ: return "GHz";
    case SC_THZ: return "THz";
  }
  return "?Hz";
}

//------------------------------------------------------------------------------
sc_freq operator*( double lhs, const sc_freq& rhs ) {
  return rhs.sc_freq::operator*(lhs);
}

//------------------------------------------------------------------------------
double operator*( const sc_time& lhs, const sc_freq& rhs ) {
  return rhs.value(SC_HZ) * lhs.to_seconds();
}

//------------------------------------------------------------------------------
double operator*( const sc_freq& lhs, const sc_time& rhs ) {
  return lhs.value(SC_HZ) * rhs.to_seconds();
}

//------------------------------------------------------------------------------
sc_freq operator/( const double& lhs, const sc_time& rhs ) {
  return  sc_freq( lhs / rhs.to_seconds() );
}

//------------------------------------------------------------------------------
sc_time operator/( double lhs, const sc_freq& rhs ) {
  double divisor { rhs.value(SC_HZ) };
  if( divisor == 0.0 ) {
    return sc_max_time();
  } else {
    return sc_time( std::abs( lhs / divisor ), sc_core::SC_SEC );
  }
}

//------------------------------------------------------------------------------
double operator/( const sc_freq& lhs, const sc_freq& rhs ) {
  return lhs.value( SC_HZ ) / rhs.value( SC_HZ );
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const sc_freq& rhs ) {
  os << rhs.to_string();
  return os;
}

}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
