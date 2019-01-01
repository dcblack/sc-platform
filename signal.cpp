#include "signal.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//   ####  ###  ####  #     #    #    #                                           
//  #    #  #  #    # ##    #   # #   #                                           
//  #       #  #      # #   #  #   #  #                                           
//   ####   #  #  ### #  #  # #     # #                                           
//       #  #  #    # #   # # ####### #                                           
//  #    #  #  #    # #    ## #     # #                                           
//   ####  ###  ####  #     # #     # #####                                       
//
////////////////////////////////////////////////////////////////////////////////
#include "report.hpp"
#include "summary.hpp"
#include <set>
#include <systemc>
#include <sysc/utils/sc_stop_here.h>
using namespace sc_core;


// WARNING: This may need modification to work with Windows!

namespace {
  const char* const MSGID{ "/Doulos/Example/Signal" };

  void my_stop( int sig )
  {
    static int level = 0;
    if( level++ ) sc_set_stop_mode(SC_STOP_IMMEDIATE);
    else          sc_stop();
  }
  void my_abort( int sig )
  {
    REPORT( FATAL, "Aborting on signal " << sig );
  }
}
void my_interrupt( int sig )
{
   INFO( ALWAYS, "Interrupted" );
   sc_interrupt_here( "Interrupted", SC_INFO );
}

std::forward_list<Signal*> Signal::s_handlers;

Signal::Signal( int sig, Handler_t sighandler )
: m_sig( sig )
, m_sighandler( sighandler )
{
  install();
  s_handlers.push_front( this );
}

Signal::~Signal( void )
{
  revert();
  if( not s_handlers.empty() ) {
    s_handlers.pop_front();
  }
}

void Signal::remove( void )
{
  for( const auto& sig: m_siglist) {
    signal( sig, SIG_DFL );
  }
}

void Signal::revert( void )
{
  remove();
  if( not s_handlers.empty() ) {
    Signal* prev = s_handlers.front();
    prev->install();
  }
}

void Signal::install( void )
{
  switch( m_sig ) {
    default: {
      if( m_sighandler != nullptr ) {
        add( m_sig, m_sighandler );
      }
      else {
        REPORT( ERROR, "Invalid signal " << m_sig << " specified." );
      }
    }
    case ABORT: {
      if( m_sighandler == nullptr ) m_sighandler = &my_abort;
      add( SIGINT,  m_sighandler );
      add( SIGABRT, m_sighandler );
    }
    case STOP: {
      if( m_sighandler == nullptr ) m_sighandler = &my_stop;
      add( SIGINT,  m_sighandler );
      add( SIGABRT, m_sighandler );
    }
    case INTERRUPT: {
      if( m_sighandler == nullptr ) m_sighandler = &my_interrupt;
      add( SIGINT,  m_sighandler );
      add( SIGABRT, m_sighandler );
    }
  }
}

void Signal::add( int sig, Handler_t sighandler )
{
  m_siglist.push_front( sig );
  signal( sig, sighandler );
}

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
#ifdef SIGNAL_EXAMPLE
// This serves both as an example and a simple unit test
#include "signal.hpp"
#include "report.hpp"
#include "summary.hpp"
#include "common.hpp"
#include <cstdio>
#include <chrono>
#include <thread>
#include <array>

using namespace std;

std::array<size_t,2> count_array;

//------------------------------------------------------------------------------
extern "C" void Handler1( int )
{
  printf("Handler1 %lu\n", ++count_array.at(0) );
}

//------------------------------------------------------------------------------
extern "C" void Handler2( int )
{
  printf("Handler2 %lu\n", ++count_array.at(1) ) ;
}

//------------------------------------------------------------------------------
SC_MODULE( Top_module )
{

  //----------------------------------------------------------------------------
  void tick( size_t n=1 )
  {
    while( n-- ) {
      INFO( MEDIUM, "Tick at " << sc_time_stamp() );
      wait(10_ns);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  //----------------------------------------------------------------------------
  void test_thread( void )
  {
    const size_t LIMIT = 3;
    const size_t TICKS = 3;
    count_array.fill(0);
    Signal SIG1{ Signal::INTERRUPT, &Handler1 };
    for(;;) {
      INFO( MEDIUM, "Watching SIG1 at " << sc_time_stamp() );
      tick(5);
      if( (count_array.at(0) & 0b11) == LIMIT ) {
        INFO( MEDIUM, "Watching SIG2 at " << sc_time_stamp() );
        Signal SIG2{ Signal::INTERRUPT, &Handler2 };
        tick(5);
      }
      if( ( count_array.at(0) >= LIMIT ) and ( count_array.at(1) >= LIMIT ) ) {
        INFO( MEDIUM, "Watching STOPPER at " << sc_time_stamp() );
        Signal STOPPER{ Signal::STOP };
        tick(5);
      }
    }
  }

  //----------------------------------------------------------------------------
  SC_CTOR( Top_module ) {
    SC_THREAD( test_thread );
  }

  //----------------------------------------------------------------------------
  void start_of_simulation( void ) override {
    Summary::starting_simulation();
  }

  //----------------------------------------------------------------------------
  void end_of_simulation( void ) override {
    Summary::finished_simulation();
  }
};

//------------------------------------------------------------------------------
int sc_main( int argc, char* argv[] )
{
  for ( int i = 1; i < sc_core::sc_argc(); ++i ) {
    std::string arg( sc_core::sc_argv()[i] );
    if( arg == "-debug" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_DEBUG );
      SC_REPORT_INFO( MSGID, "Verbosity level set to DEBUG" );
    }
  }
  sc_report_handler::set_actions( SC_ERROR, SC_DISPLAY | SC_LOG );
  Summary::starting_elaboration();
  Top_module top( "top" );
  sc_start();
  if ( ! sc_end_of_simulation_invoked() ) sc_stop();
  return Summary::report();
}
#endif
////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
// END $Id: signal.cpp,v 1.0 2018/11/19 05:18:06 dcblack Exp $
