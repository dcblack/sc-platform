//FILE: timer_beh.cpp (systemc)

// DESCRIPTION
//   This implements the behavior of a generic timer function.

#include "timer_beh.h"
#include "boost/format.hpp"
using boost::format;
using namespace sc_core;

void Timer::start( void )
{
  if( m_paused ) {
    resume();
  } else {
    m_start_time = curr_time();
    m_timeout_time = curr_time() + m_load_time;
    m_timeout_event.notify( m_load_time );
  }
}

void Timer::stop( void )
{
  sc_assert( m_paused == false );
  m_timeout_event.cancel();
  m_timeout_time = SC_ZERO_TIME;
}

void Timer::pause( void )
{
  m_paused = true;
  m_timeout_event.cancel();
  m_resume_delay = m_timeout_time - curr_time();
}

void Timer::resume( void )
{
  m_paused = false;
  m_timeout_event.notify( m_resume_delay );
  m_resume_delay = SC_ZERO_TIME;
}

sc_time Timer::curr_time( void )
{
  return sc_time_stamp();
}

// To find out how much time is left on the clock
sc_time timer::get_time_left(sc_time tLOCAL) const {
  return (m_timeout_time > sc_time_stamp()+tLOCAL) //< is it in the future?
    ? (m_timeout_time - sc_time_stamp()) //< amount of time left
    : SC_ZERO_TIME; //< no time left
}

// Call the following at timeout_event's
bool timer::timeout(void) { 
  if (m_timeout_time != SC_ZERO_TIME 
  and m_timeout_time == curr_time())
  {
    m_expired = true;
    if (m_reload and m_initial_delay != SC_ZERO_TIME) set_time(m_initial_delay);
    return true;
  } else {
    return false;
  }
}

//EOF
