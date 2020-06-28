//FILE main.cpp (systemc)
// $Info: Entry point for executing simulation for 'plat'.$
//-----------------------------------------------------------------------------
#include "top/top.hpp"
#include <tlm>
#include <memory>
#include "top/wallclock.hpp"
#include "report/report.hpp"
#include "report/summary.hpp"
#include "top/news.hpp"

namespace {
  char const * const MSGID{ "/Doulos Inc/Example/main" };
}

using namespace sc_core;
using namespace std;

//------------------------------------------------------------------------------
int sc_main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
  unique_ptr<Top_module> top;
  News improved;
  // Elaborate
  Summary::starting_elaboration();

  try {
    top.reset( new Top_module( "top" ) );
  }
  catch ( sc_exception& e )
  {
    REPORT( INFO, "\n" << e.what() << "\n\n*** Please fix elaboration errors and retry. ***" );
    return Summary::report();
  }
  catch ( ... )
  {
    REPORT( INFO, "Error: *** Caught unknown exception during elaboration. ***" );
    Summary::increment_errors();
    return Summary::report();
  }//endtry

  if( Summary::errors() != 0 )
  {
    return Summary::report();
  }

  // Simulate
  try {
    REPORT( INFO, "Starting kernel" );
    Summary::starting_simulation(); //< best we can do
    sc_start();
    REPORT( INFO, "Exited kernel at " << sc_time_stamp() );
    Summary::finished_simulation();
  }
  catch ( sc_exception& e )
  {
    REPORT( WARNING, "Caught exception during active simulation.\n" << e.what() );
  }
  catch ( ... )
  {
    REPORT( WARNING, "Error: Caught unknown exception during active simulation." );
    Summary::increment_errors();
  }//endtry

  //--------------------------------------------------------------------------
  // Clean up
  //--------------------------------------------------------------------------
  if ( not sc_end_of_simulation_invoked() )
  {
    REPORT( INFO, "\nError: Simulation stopped without explicit sc_stop()" );
    Summary::increment_errors();

    try {
      sc_stop(); //< this will invoke end_of_simulation() callbacks
      Summary::finished_simulation(); // update
    }
    catch ( sc_exception& e )
    {
      REPORT( WARNING, "Caught exception while stopping.\n" << e.what() );
    }
    catch(...) {
      REPORT( WARNING, "Error: Caught unknown exception while stopping." );
      Summary::increment_errors();
    }
  }//endif

  return Summary::report();

}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
// -*- C++ -*- vim:sw=2:tw=0:et
//END main.cpp $Id$ >>>}
