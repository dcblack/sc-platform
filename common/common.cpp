#include "common/common.hpp"
#include <sstream>
#include <string>
using namespace sc_core;

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
    case Style::      TD: os << "LT+TD";   break;
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
