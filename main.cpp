//FILE main.cpp (systemc)
// $Info: Entry point for executing simulation for 'plat'.$
//-----------------------------------------------------------------------------
#include "top.hpp"
#include <tlm>
#include "wallclock.hpp"
#if __cplusplus >= 201103L
#  include <memory>
#endif
#include "report.hpp"
std::ostringstream mout;

namespace {
  const char* MSGID = "/Doulos Inc/Example/main";
  double elaboration_time = -1.0, starting_time = -1.0, finished_time = -1.0;
  unsigned int main_errors = 0; //< locally detected
  unsigned int main_warnings = 0; //< locally detected
  int summary( void );
}

using namespace sc_core;
using namespace std;

//------------------------------------------------------------------------------
int sc_main(int argc, char *argv[])
{
  unique_ptr<Top_module> top;
  // Elaborate
  elaboration_time = get_cpu_time();

  try {
    top.reset( new Top_module( "top" ) );
  }
  catch ( sc_exception& e )
  {
    REPORT( INFO, "\n" << e.what() << "\n\n*** Please fix elaboration errors and retry. ***" );
    return summary();
  }
  catch ( exception& e )
  {
    REPORT( INFO, "\n" << e.what() << "\n\n*** Please fix elaboration errors and retry. ***" );
    return summary();
  }
  catch ( ... )
  {
    REPORT( INFO, "Error: *** Caught unknown exception during elaboration. ***" );
    ++main_errors;
    return summary();
  }//endtry

  // Simulate
  try {
    REPORT( INFO, "Starting kernel" );
    starting_time = get_cpu_time();
    sc_start();
    finished_time = get_cpu_time();
    REPORT( INFO, "Exited kernel at " << sc_time_stamp() );
  }
  catch ( sc_exception& e )
  {
    REPORT( WARNING, "Caught exception during active simulation.\n" << e.what() );
  }
  catch ( exception& e )
  {
    REPORT( WARNING, "Caught exception during active simulation.\n" << e.what() );
  }
  catch ( ... )
  {
    REPORT( WARNING, "Error: Caught unknown exception during active simulation." );
    ++main_errors;
  }//endtry

  // Clean up

  if ( ! sc_end_of_simulation_invoked() )
  {
    try {
      REPORT( INFO, "\nError: Simulation stopped without explicit sc_stop()" );
      ++main_errors;
    }
    catch ( sc_exception& e ) {
      REPORT( INFO, "\n\n" << e.what() );
    }

    sc_stop(); //< this will invoke end_of_simulation() callbacks
  }//endif

  return summary();

}

//-----------------------------------------------------------------------------
namespace {
  // Summarize results
  int summary( void )
  {
    {
      bool warn{ false };
      MESSAGE( "\n" );
      RULER( '#' );
      MESSAGE( "Compilation information for " << sc_argv()[0] << ":\n" );
      MESSAGE( "  C++     version: " << __cplusplus << "\n" );
      MESSAGE( "  SystemC version: " << SYSTEMC_VERSION << "\n" );
      MESSAGE( "  TLM     version: " << TLM_VERSION << "\n" );
      #ifdef BOOST_LIB_VERSION
      MESSAGE( "  BOOST   version: " << BOOST_LIB_VERSION << "\n" );
      #endif
      #if defined(__clang__)
      MESSAGE( "Compiled with Clang/LLVM version " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << "\n" );
      #elif defined(__ICC) || defined(__INTEL_COMPILER)
      MESSAGE( "Compiled with Intel ICC/ICPC version " << __ICC << "\n" );
      #elif defined(__GNUC__) || defined(__GNUG__)
      MESSAGE( "Compiled with GNU GCC/G++ version " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n" );
      #elif defined(__HP_cc) || defined(__HP_aCC)
      MESSAGE( "Compiled with Hewlett-Packard C/aC++ version " << __HP_cc << "\n" );
      #elif defined(__IBMC__) || defined(__IBMCPP__)
      MESSAGE( "Compiled with IBM XL C/C++ version " << __IBMCPP__ << "\n" );
      #elif defined(_MSC_VER)
      MESSAGE( "Compiled with Microsoft Visual Studio version " << _MSC_FULL_VER << "\n" );
      #elif defined(__PGI)
      MESSAGE( "Compiled with Portland Group PGCC/PGCPP version " << __PGIC__ << "." << __PGIC_MINOR__ << "." << __PGIC_PATCHLEVEL__ << "\n" );
      #elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
      MESSAGE( "Compiled with Oracle Solaris Studio version " << std::hex << __SUNPRO_CC << "\n" );
      #else
      MESSAGE( "Unknown compiler!" << "\n" );
      ++main_warnings;
      warn = true;
      #endif
      MEND( ALWAYS );
      if( warn ) {
        REPORT( WARNING, " Please inform Doulos of your compiler and how to check for its existance." );
      }
    }
    string kind = "Simulation";

    if ( starting_time < 0.0 ) {
      kind = "Elaboration";
      starting_time = finished_time = get_cpu_time();
    }

    if ( finished_time < 0.0 ) {
      finished_time = get_cpu_time();
    }

    auto errors = sc_report_handler::get_count( SC_ERROR )
                  + main_errors
                  + sc_report_handler::get_count( SC_FATAL );
    MESSAGE( "\n" );
    RULER( '-' );
    MESSAGE( "Summary for " << sc_argv()[0] << ":\n" );
    MESSAGE( "  CPU elaboration time " << setprecision( 4 ) << ( starting_time - elaboration_time ) << " sec\n" );
    MESSAGE( "  CPU simulation  time " << setprecision( 4 ) << ( finished_time - starting_time ) << " sec\n" );
    MESSAGE( "  " << setw( 2 ) << sc_report_handler::get_count( SC_WARNING )             << " warnings" << "\n" );
    MESSAGE( "  " << setw( 2 ) << sc_report_handler::get_count( SC_ERROR ) + main_errors << " errors"   << "\n" );
    MESSAGE( "  " << setw( 2 ) << sc_report_handler::get_count( SC_FATAL )               << " fatals"   << "\n" );
    MESSAGE( "\n" );
    RULER( '#' );
    MESSAGE( kind << " " << ( errors ? "FAILED" : "PASSED" ) );
    MEND( ALWAYS );
    return ( errors ? 1 : 0 );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
// -*- C++ -*- vim:sw=2:tw=0:et
//END main.cpp $Id$ >>>}
