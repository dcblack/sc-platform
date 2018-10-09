#include "util.hpp"
using namespace sc_core;
using namespace std;
using namespace tlm;

string to_string( tlm_command command )
{
  switch( command )
  {
    case  TLM_WRITE_COMMAND: return  "TLM_WRITE_COMMAND";
    case   TLM_READ_COMMAND: return   "TLM_READ_COMMAND";
    case TLM_IGNORE_COMMAND: return "TLM_IGNORE_COMMAND";
    default:                 return   "!UKNOWN_COMMAND!";
  }
}

//------------------------------------------------------------------------------
#include <random>
sc_time rand_ps( double mean, double stddev )
{
  sc_assert( mean > 0.0 and stddev >= 0.0 );
  sc_assert( stddev < mean );
  if ( stddev == 0.0 ) {
    stddev = 0.25 * mean;
  }
  static std::default_random_engine generator;
  std::normal_distribution<double> distribution( mean, stddev );
  double time = distribution( generator );
  return sc_time( time, SC_PS );
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
