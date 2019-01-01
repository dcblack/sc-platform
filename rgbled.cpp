#include "rgbled.hpp"
#include "report.hpp"
#include "sc_cxx11.hpp"
#include "ansi.hpp"
using namespace sc_core;
using namespace sc_dt;
namespace {
  const char* const MSGID{ "/Doulos/Example/RgbLED" };
}

//------------------------------------------------------------------------------
// Constructor
RgbLED_module::RgbLED_module(sc_module_name instance_name)
: sc_module(instance_name)
{
  // Connectivity - NONE
  // Register processes
  SC_HAS_PROCESS(RgbLED_module);
  SC_METHOD(rgbled_method);
  sensitive << r << g << b;
}//endconstructor

//------------------------------------------------------------------------------
// Destructor
RgbLED_module::~RgbLED_module( void ) {
}

//------------------------------------------------------------------------------
// Processes
void RgbLED_module::rgbled_method( void ) {

  int rgb = ((  r->read() == SC_LOGIC_1 ) ? 4 : 0 )
          + ((  g->read() == SC_LOGIC_1 ) ? 2 : 0 )
          + ((  b->read() == SC_LOGIC_1 ) ? 1 : 0 )
          ;
  MESSAGE(  name() << " is " << Ansi::bold );
  switch( rgb ) {
    case 0:  MESSAGE( Ansi::bg_black   << Ansi::fg_white << "BLACK"   ); break;
    case 1:  MESSAGE( Ansi::bg_blue    << Ansi::fg_white << "BLUE"    ); break;
    case 2:  MESSAGE( Ansi::bg_green   << Ansi::fg_white << "GREEN"   ); break;
    case 3:  MESSAGE( Ansi::bg_cyan    << Ansi::fg_black << "CYAN"    ); break;
    case 4:  MESSAGE( Ansi::bg_red     << Ansi::fg_white << "RED"     ); break;
    case 5:  MESSAGE( Ansi::bg_magenta << Ansi::fg_white << "MAGENTA" ); break;
    case 6:  MESSAGE( Ansi::bg_yellow  << Ansi::fg_black << "YELLOW"  ); break;
    case 7:  MESSAGE( Ansi::bg_white   << Ansi::fg_black << "WHITE"   ); break;
    default: MESSAGE( Ansi::fg_red << "UNKNOWN" ); break;
  }
  MESSAGE(  Ansi::reset );
  MEND( ALWAYS );

}//end RgbLED_module::rgbled_method


#ifdef RGBLED_EXAMPLE
////////////////////////////////////////////////////////////////////////////////
//
//  ##### #     #    #    #     # #####  #     #####                             
//  #      #   #    # #   ##   ## #    # #     #                                 
//  #       # #    #   #  # # # # #    # #     #                                 
//  #####    #    #     # #  #  # #####  #     #####                             
//  #       # #   ####### #     # #      #     #                                 
//  #      #   #  #     # #     # #      #     #                                 
//  ##### #     # #     # #     # #      ##### #####                             
//
////////////////////////////////////////////////////////////////////////////////
// This serves both as an example and a simple unit test
#include "rgbled.hpp"
#include "report.hpp"
#include "summary.hpp"
#include "no_clock.hpp"
#include <chrono>
#include <thread>

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

//------------------------------------------------------------------------------
SC_MODULE( Top_module )
{
  no_clock clk      { "clk", 10_ns };
  RgbLED_module led1{ "led1" };
  sc_vector<sc_signal<sc_logic>> v{ "v", 3 };

  //----------------------------------------------------------------------------
  void test_thread( void )
  {
    for( int i=0; i<16; ++i ) {
      sleep_for( 500ms );
      wait( 500_ms );
      v[0].write( (i&1) ? SC_LOGIC_1 : SC_LOGIC_0 );
      v[1].write( (i&2) ? SC_LOGIC_1 : SC_LOGIC_0 );
      v[2].write( (i&4) ? SC_LOGIC_1 : SC_LOGIC_0 );
    }
    wait( 10_ms );
    sc_stop();
  }

  //----------------------------------------------------------------------------
  SC_CTOR( Top_module ) {
    SC_HAS_PROCESS( Top_module );
    SC_THREAD( test_thread );
    led1.r.bind( v[0] );
    led1.g.bind( v[1] );
    led1.b.bind( v[2] );
  }

  //----------------------------------------------------------------------------
  void start_of_simulation( void ) override {
    Summary::starting_simulation();
  }

  //----------------------------------------------------------------------------
  void end_of_simulation( void ) override {
    Summary::finished_simulation();
  }

  //----------------------------------------------------------------------------
  const char* kind() const override {
    return "Top_module";
  }
  // Attributes
  static constexpr char const * const MSGID
  { "/Doulos/Example/Rgbled_example" };
};

//------------------------------------------------------------------------------
int sc_main( int argc, char* argv[] )
{
  for ( int i = 1; i < sc_core::sc_argc(); ++i ) {
    std::string arg( sc_core::sc_argv()[i] );
    if( arg == "-debug" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_DEBUG );
      SC_REPORT_INFO( Top_module::MSGID, "Verbosity level set to DEBUG" );
    }
  }
  sc_report_handler::set_actions( SC_ERROR, SC_DISPLAY | SC_LOG );
  Summary::starting_elaboration();
  Top_module top( "top" );
  sc_start();
  return Summary::report();
}
#endif

//------------------------------------------------------------------------------
// Copyright 2018 by Doulos. All rights reserved.
//END rgbled.cpp @(#)$Id$
