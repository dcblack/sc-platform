#include "tcpip/tcpip_tx.hpp"

// Nothing here except syntax check and self-test/example

#ifdef TCPIP_EXAMPLE

using namespace std;
using namespace sc_core;
#include "tcpip/tcpip_tx.hpp"
#include "report/report.hpp"
#include "report/summary.hpp"
#include <iostream>

SC_MODULE( Top_module )
{
  char const * const MSGID{ "/Doulos/Example/Config_example" };

  //----------------------------------------------------------------------------
  SC_CTOR( Top_module ) {
    SC_THREAD( test_thread );
  }

  //----------------------------------------------------------------------------
  void test_thread( void ) {
    string text = "Hello\nworld\n!\n";
    Async_payload<string> msg{ Asynk_kind::stream, text };
    tx.put( msg );
    msg.set_data( string("Goodbye!") );
    tx.put( msg );
    sc_stop();
  }

  // Local channels
  Tcpip_tx_channel tx{ "Tx", 0xE5C0ul };

  // Attributes
};

int sc_main( int argc, char* argv[] )
{
  for(int i=1; i<sc_core::sc_argc(); ++i) {
    std::string arg(sc_core::sc_argv()[i]);
    if (arg == "-debug") {
      sc_core::sc_report_handler::set_verbosity_level(SC_DEBUG);
      SC_REPORT_INFO( "/Doulos/Example/config_example", "Verbosity level set to DEBUG" );
    }
  }

  //----------------------------------------------------------------------------
  // Elaborate
  //----------------------------------------------------------------------------
  Summary::starting_elaboration();
  Top_module top( "top" );
  if( Summary::errors() != 0 )
  {
    return Summary::report();
  }

  //----------------------------------------------------------------------------
  // Begin simulator
  //----------------------------------------------------------------------------
  sc_start();

  //----------------------------------------------------------------------------
  // Clean up
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
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
}//endif
  return Summary::report();
}
#endif

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END tcpip_tx.cpp @(#)$Id$
