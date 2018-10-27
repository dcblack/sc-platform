//FILE: timer_beh.cpp (systemc)

// DESCRIPTION
//   This implements the behavior of a generic timer function.

#include "timer_beh.hpp"
#include "report.hpp"

using namespace sc_core;

// Class static constants
const char* Timer::MSGID { "/Doulos/example/timer_beh" };

//------------------------------------------------------------------------------
// Constructor
Timer::Timer( sc_module_name nm )
  : sc_module( nm )
{
  SC_HAS_PROCESS( Timer );
  SC_THREAD( trigger_thread );
  INFO( MEDIUM, "Default constructed " << __func__ );
}

//------------------------------------------------------------------------------
// Destructor
Timer::~Timer( void )
{
  INFO( MEDIUM, "Destroyed " << __func__ );
}

//------------------------------------------------------------------------------
void Timer::start( sc_time delay )
{
  if ( m_paused ) {
    resume();
  }
  else {
    m_start_time = curr_time( delay );
    m_trigger_time = m_start_time + m_trig_delay;
    m_trigger_event.notify( m_trig_delay + delay );
    INFO( DEBUG, "Timer " << name() << " started." );
  }
}

//------------------------------------------------------------------------------
void Timer::stop( sc_time delay )
{
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
  m_paused = true;
  m_trigger_event.cancel();
  m_resume_delay = m_trigger_time - curr_time( delay );
  INFO( DEBUG, "Timer " << name() << " paused." );
}

//------------------------------------------------------------------------------
void Timer::resume( sc_time delay )
{
  sc_assert( m_paused and m_resume_delay != SC_ZERO_TIME );
  m_paused = false;
  m_trigger_time = curr_time( delay ) + m_resume_delay;
  m_start_time = m_trigger_time - m_trig_delay;
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
  if ( m_trigger_time > curr_time(delay) ) { //< is it in the future?
    return ( m_trigger_time - curr_time(delay) ); //< amount of time left
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
  running &= ( get_time_left( delay ) == SC_ZERO_TIME ) and not m_continuous;
  return running;
}

//------------------------------------------------------------------------------
void Timer::trigger_thread( void )
{
  wait( m_trigger_event );
  sc_assert ( m_trigger_time != SC_ZERO_TIME
       and m_trigger_time == sc_time_stamp()
       and not m_paused );

  m_triggered = true;

  if ( m_reload and m_trig_delay != SC_ZERO_TIME ) {
    m_start_time = curr_time( );
  }

  if ( m_continuous ) {
    m_trigger_time = curr_time( ) + m_trig_delay;
    m_trigger_event.notify( m_trig_delay );
  } else {
    m_trigger_time = SC_ZERO_TIME;
  }
}

//EOF
