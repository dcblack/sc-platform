#include "common.hpp"
#include <sstream>
#include <string>
using namespace sc_core;

std::string verbosity2str(const int & level)
{
  std::ostringstream os;
  switch( level ) {
    case SC_NONE   : os << "NONE";   break;
    case SC_LOW    : os << "LOW";    break;
    case SC_MEDIUM : os << "MEDIUM"; break;
    case SC_HIGH   : os << "HIGH";   break;
    case SC_FULL   : os << "FULL";   break;
    case SC_DEBUG  : os << "DEBUG";  break;
    default:
      if(      level < SC_LOW    ) { os << std::to_string( int( level - SC_NONE   ) ) << "% LOW"    ; }
      else if( level < SC_MEDIUM ) { os << std::to_string( int( level - SC_LOW    ) ) << "% MEDIUM" ; }
      else if( level < SC_HIGH   ) { os << std::to_string( int( level - SC_MEDIUM ) ) << "% HIGH"   ; }
      else if( level < SC_FULL   ) { os << std::to_string( int( level - SC_HIGH   ) ) << "% FULL"   ; }
      else if( level < SC_DEBUG  ) { os << std::to_string( int( level - SC_FULL   ) ) << "% DEBUG"  ; }
      else                         { os << "DEBUG + " << std::to_string( int( level - SC_DEBUG ) )  ; }
  }//endswitch
  return os.str();
}

std::ostream& operator<<(std::ostream& os, const Access& rhs)
{
  switch( rhs ) {
    case Access::   RW: os << "RW"  ; break;
    case Access::   RO: os << "RO"  ; break;
    case Access::   WO: os << "WO"  ; break;
    case Access::   RC: os << "RC"  ; break;
    case Access::   RS: os << "RS"  ; break;
    case Access::  W1C: os << "W1C" ; break;
    case Access::  W0C: os << "W0C" ; break;
    case Access::  W1S: os << "W1S" ; break;
    case Access::  W0S: os << "W0S" ; break;
    case Access::  W1T: os << "W1T" ; break;
    case Access::  W0T: os << "W0T" ; break;
    case Access::  RAZ: os << "RAZ" ; break;
    case Access:: none: os << "none"; break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Feature& rhs)
{
  os << ( rhs == Feature::enabled ? "enabled" : "none" );
  return os;
}

std::ostream& operator<<(std::ostream& os, const Style& rhs)
{
  switch( rhs ) {
    case Style::      AT: os << "AT";      break;
    case Style::      LT: os << "LT";      break;
    case Style:: DEFAULT: os << "DEFAULT"; break;
    case Style:: UNKNOWN: os << "UNKNOWN"; break;
  }
  return os;
}

bool  g_error_at_target { false };
Style g_coding_style    { Style::LT };

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
