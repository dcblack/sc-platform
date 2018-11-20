//FILE: timer_beh.cpp (systemc)

// DESCRIPTION
//   This implements the behavior of a generic timer function.

#include "timer_beh.hpp"
#include "sc_cxx11.hpp"
#include "report.hpp"

using namespace sc_core;
namespace {
  const char* const MSGID { "/Doulos/example/timer_beh" };
}

//------------------------------------------------------------------------------
// Constructor
Timer::Timer( sc_module_name nm )
  : sc_module( nm )
  , m_pulse_delay(1_ps)
{
  SC_HAS_PROCESS( Timer );
  SC_THREAD( trigger_thread );
  INFO( MEDIUM, "Constructed " << name() );
}

//------------------------------------------------------------------------------
// Destructor
Timer::~Timer( void )
{
  INFO( MEDIUM, "Destroyed " << name() );
}

//------------------------------------------------------------------------------
void Timer::start( sc_time delay )
{
  INFO( DEBUG, "Starting timer " << name() );
  if( m_paused ) {
    resume();
  }
  else {
    m_start_time = curr_time( delay );
    m_trigger_time = m_start_time + m_load_delay;
    m_trigger_event.notify( m_load_delay + delay );
    INFO( DEBUG, "Timer " << name() << " started." );
  }
}

//------------------------------------------------------------------------------
void Timer::stop( sc_time delay )
{
  INFO( DEBUG, "Stopping timer " << name() );
  // Cancel outstanding event
  m_trigger_event.cancel();
  m_trigger_time = SC_ZERO_TIME;
  // Override pause
  m_paused = false;
  m_resume_delay = SC_ZERO_TIME;
  INFO( DEBUG, "Timer " << name() << " stopped." );
}

//------------------------------------------------------------------------------
void Timer::pause( sc_time delay )
{
  INFO( DEBUG, "Pausing timer " << name() );
  m_paused = true;
  m_trigger_event.cancel();
  m_resume_delay = m_trigger_time - curr_time( delay );
  INFO( DEBUG, "Timer " << name() << " paused." );
}

//------------------------------------------------------------------------------
void Timer::resume( sc_time delay )
{
  INFO( DEBUG, "Resuming timer " << name() );
  sc_assert( m_paused and m_resume_delay != SC_ZERO_TIME );
  m_paused = false;
  m_trigger_time = curr_time( delay ) + m_resume_delay;
  m_start_time = m_trigger_time - m_load_delay;
  m_trigger_event.notify( m_resume_delay + delay );
  m_resume_delay = SC_ZERO_TIME;
  INFO( DEBUG, "Timer " << name() << " resumed." );
}

//------------------------------------------------------------------------------
sc_time Timer::curr_time( sc_time delay ) const
{
  return sc_time_stamp() + delay;
}

//------------------------------------------------------------------------------
// To find out how much time is left on the clock
sc_time Timer::get_time_left( sc_time delay ) const
{
  if( m_trigger_time > curr_time(delay) ) { //< is it in the future?
    return ( m_trigger_time - curr_time(delay) ); //< amount of time left
  }
  else {
    return SC_ZERO_TIME; //< no time left
  }
}

//------------------------------------------------------------------------------
// To find out how much time since timer started
sc_time Timer::get_time_since( sc_time delay ) const
{
  if( m_start_time > SC_ZERO_TIME and m_start_time < curr_time(delay) ) {
    return curr_time(delay) - m_start_time;
  }
  else {
    return SC_ZERO_TIME; //< no time left
  }
}

//------------------------------------------------------------------------------
bool Timer::is_running( sc_core::sc_time delay )
{
  bool running = not m_paused;
  running &= ( m_trigger_time != SC_ZERO_TIME );
  running &= ( get_time_left( delay ) > SC_ZERO_TIME ) or m_continuous;
  return running;
}

//------------------------------------------------------------------------------
void Timer::trigger_thread( void )
{
  for(;;) {
    wait( m_trigger_event );
    sc_assert ( m_trigger_time != SC_ZERO_TIME
         and m_trigger_time == sc_time_stamp()
         and not m_paused );

    m_triggered = true;
    if( m_pulse_delay > SC_ZERO_TIME ) {
      m_pulse_event.notify( m_pulse_delay );
    }

    if( m_reload and m_load_delay != SC_ZERO_TIME ) {
      m_start_time = sc_time_stamp( );
    }

    if( m_continuous ) {
      m_trigger_time = sc_time_stamp( ) + m_load_delay;
      m_trigger_event.notify( m_load_delay );
    } else {
      m_trigger_time = SC_ZERO_TIME;
    }
  }
}

//------------------------------------------------------------------------------
std::string Timer::to_string( void )
{
  std::ostringstream os;
  os << "timer state: {\n"
     << "-  current time: " << sc_time_stamp() << "\n"
     << "-    load delay: " << get_load_delay() << "\n"
     << "-   pulse delay: " << get_pulse_delay() << "\n"
     << "-    start time: " << get_start_time() << "\n"
     << "-  trigger time: " << get_trigger_time() << "\n"
     << "-     time left: " << get_time_left() << "\n"
     << "-    time since: " << get_time_since() << "\n"
     << "-  trigger flag: " << (get_triggered()?"TRIGGERED":"-") << "\n"
     << "-     mode flag: " << (get_continuous()?"CONTINUOUS":"one-shot") << "\n"
     << "-   reload flag: " << (get_reload()?"ACTIVE":"off") << "\n"
     << "- running state: " << (is_running()?"RUNNING":"off") << "\n"
     << "-  paused state: " << (is_paused()?"PAUSED":"-") << "\n"
     << "}" << std::ends
  ;
  return os.str();
}

//------------------------------------------------------------------------------
void Timer::reset( void ) {
  m_trigger_event.cancel();
  m_pulse_event.cancel();
  m_load_delay   = sc_core::SC_ZERO_TIME;
  m_start_time   = sc_core::SC_ZERO_TIME;
  m_pulse_delay  = sc_core::SC_ZERO_TIME;
  m_resume_delay = sc_core::SC_ZERO_TIME;
  m_trigger_time = sc_core::SC_ZERO_TIME;
  m_triggered    = false;
  m_reload       = false;
  m_continuous   = false;
  m_paused       = false;
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
#ifdef TIMER_EXAMPLE
// This serves both as an example and a simple unit test
#include "timer_beh.hpp"
#include "report.hpp"
#include "summary.hpp"
#include "common.hpp"
#include "no_clock.hpp"
#include <queue>
#include <iostream>

using namespace std;

//------------------------------------------------------------------------------
SC_MODULE( Top_module )
{
  const char* MSGID{ "/Doulos/Example/Timer_example" };
  no_clock clk   { "clk", 10_ns };

  //----------------------------------------------------------------------------
  void test_thread( void )
  {
    // Display reset state
    MESSAGE("\n");
    RULER('R');
    INFO( MEDIUM, "Reset " << timer.to_string() );

    // Test one shot
    MESSAGE("\n");
    RULER('1');
    clk.wait_posedge(1);
    timer.set_load_delay( 10_ns );
    timer.set_one_shot();
    timer.set_reload( false );
    INFO( MEDIUM, "One-shot setup " << timer.to_string() );
    expect.push( sc_time_stamp() + 10_ns );
    timer.start( SC_ZERO_TIME );
    wait(5_ns);
    INFO( MEDIUM, "One-shot midway " << timer.to_string() );
    wait(6_ns);
    INFO( MEDIUM, "One-shot finished " << timer.to_string() );

    // Test one shot with 1ns pause
    MESSAGE("\n");
    RULER('P');
    clk.wait_posedge();
    timer.set_load_delay( 10_ns );
    timer.set_one_shot();
    timer.set_reload( false );
    INFO( MEDIUM, "One-shot setup " << timer.to_string() );
    expect.push( sc_time_stamp() + 21_ns );
    timer.start( SC_ZERO_TIME );
    wait(5_ns);
    INFO( MEDIUM, "One-shot midway " << timer.to_string() );
    timer.pause();
    INFO( MEDIUM, "One-shot paused " << timer.to_string() );
    wait(11_ns);
    INFO( MEDIUM, "One-shot resuming " << timer.to_string() );
    timer.resume();
    wait(6_ns);
    INFO( MEDIUM, "One-shot finished " << timer.to_string() );

    // Test re-load
    MESSAGE("\n");
    RULER('L');
    clk.wait_posedge();
    timer.set_load_delay( 5_ns );
    timer.set_continuous();
    timer.set_reload( true );
    expect.push( sc_time_stamp() +  5_ns );
    expect.push( sc_time_stamp() + 10_ns );
    expect.push( sc_time_stamp() + 15_ns );
    timer.start( SC_ZERO_TIME );
    INFO( MEDIUM, "Reload setup " << timer.to_string() );
    wait(3_ns);
    INFO( MEDIUM, "Reload running " << timer.to_string() );
    wait(5_ns);
    INFO( MEDIUM, "Reload running " << timer.to_string() );
    wait(10_ns);
    INFO( MEDIUM, "Reload finished " << timer.to_string() );

    // Test reset
    timer.reset();
    INFO( MEDIUM, "Reset " << timer.to_string() );

    while ( not expect.empty() ) {
      REPORT( ERROR, "Missed event for " << expect.top() );
      expect.pop();
      ++missed_count;
    }
    sc_stop();
  }

  //----------------------------------------------------------------------------
  void interrupt_thread( void ) {
    for(;;) {
      INFO( MEDIUM, "Waiting for timeout at " << sc_time_stamp() );
      wait( timer.trigger_event() );
      ++event_count;
      if( expect.empty() ) {
        REPORT( ERROR, "Unexpected event at " << sc_time_stamp() );
        ++unexpected_count;
      }
      else if( expect.top() == sc_time_stamp() ) {
        INFO( MEDIUM, "Expected event at " << sc_time_stamp() );
        expect.pop();
        ++expected_count;
      }
      else if( expect.top() < sc_time_stamp() ) {
        while ( not expect.empty() and expect.top() < sc_time_stamp() ) {
          REPORT( ERROR, "Missed event for " << expect.top() );
          expect.pop();
          ++missed_count;
        }
        if( expect.top() == sc_time_stamp() ) {
          INFO( MEDIUM, "Expected event at " << sc_time_stamp() );
          expect.pop();
          ++expected_count;
        }
        else {
          REPORT( ERROR, "Unexpected event at " << sc_time_stamp() );
          ++unexpected_count;
        }
      }
      else if( expect.top() > sc_time_stamp() ) {
        REPORT( ERROR, "Unexpected event at " << sc_time_stamp() );
        ++unexpected_count;
      }
    }
  }

  //----------------------------------------------------------------------------
  SC_CTOR( Top_module ) {
    SC_THREAD( test_thread );
    SC_THREAD( interrupt_thread );
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
  Timer timer{"timer"};
  size_t event_count      { 0 };
  size_t missed_count     { 0 };
  size_t expected_count   { 0 };
  size_t unexpected_count { 0 };
  std::priority_queue<sc_time, std::vector<sc_time>, std::greater<sc_time>> expect;
};

//------------------------------------------------------------------------------
int sc_main( int argc, char* argv[] )
{
  for ( int i = 1; i < sc_core::sc_argc(); ++i ) {
    std::string arg( sc_core::sc_argv()[i] );
    if( arg == "-debug" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_DEBUG );
      SC_REPORT_INFO( "/Doulos/Example/Timer_example", "Verbosity level set to DEBUG" );
    }
  }
  sc_report_handler::set_actions( SC_ERROR, SC_DISPLAY | SC_LOG );
  Summary::starting_elaboration();
  Top_module top( "top" );
  sc_start();
  return Summary::report();
}
#endif
////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
