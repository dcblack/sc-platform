#ifndef TIMER_H
#define TIMER_H
#include <systemc>
// NOTE: This class may be constructed at any time
class Timer { //< Private to hold data
public:

  Timer( void ) = default; //< Default constructor
  Timer( const Timer& ) = delete; //< Copy constructor
  Timer( Timer&& ) = delete;//< Move constructor
  Timer& operator=( const Timer& ) = delete; //< Copy assignment
  Timer& operator=( Timer&& ) = delete; //< Move assignment
  ~Timer( void ) = default; //< Destructor

  // Accessors
  void set_curr_time    ( sc_core::sc_time t ) { m_curr_time    = t; }
  void set_load_time    ( sc_core::sc_time t ) { m_load_time    = t; }
  void set_start_time   ( sc_core::sc_time t ) { m_start_time   = t; }
  void set_pulse_time   ( sc_core::sc_time t ) { m_pulse_time   = t; }
  void set_timeout_time ( sc_core::sc_time t ) { m_timeout_time = t; }
  sc_core::sc_time get_curr_time    ( void ) const { return m_curr_time;   }
  sc_core::sc_time get_load_time    ( void ) const { return m_load_time;   }
  sc_core::sc_time get_start_time   ( void ) const { return m_start_time;  }
  sc_core::sc_time get_pulse_time   ( void ) const { return m_pulse_time;  }
  sc_core::sc_time get_timeout_time ( void ) const { return m_timeout_time; }
  void set_expired    ( bool v ) { m_expired    = v; }
  void set_reload     ( bool v ) { m_reload     = v; }
  void set_continuous ( bool v ) { m_continuous = v; }
  void set_count_up   ( bool v ) { m_count_up   = v; }
  void set_overflowed ( bool v ) { m_overflowed = v; }
  void set_paused     ( bool v ) { m_paused     = v; }
  bool get_expired    ( void ) const { return m_expired;    }
  bool get_reload     ( void ) const { return m_reload;     }
  bool get_continuous ( void ) const { return m_continuous; }
  bool get_count_up   ( void ) const { return m_count_up;   }
  bool get_overflowed ( void ) const { return m_overflowed; }
  bool get_paused     ( void ) const { return m_paused;     }

  sc_core::sc_time Timer::curr_time( void );
  void start( void );
  void stop( void );
  void pause( void );
  void resume( void );
  const sc_core::sc_event& timeout_event ( void ) const { return m_timeout_event; } //< triggered when timer expires
  const sc_core::sc_event& pulse_event ( void ) const { return m_pulse_event; } //< triggered when timer expires

  sc_core::sc_time get_time_left ( sc_core::sc_time tLOCAL ) const; //< how much time remains

  bool timeout(void); //< checks for expiration and notifies event if exact time

  bool timed_out ( void ) const { return m_expired; } //< indicates if timeout event was noted

private:
  sc_core::sc_time  m_curr_time    { SC_ZERO_TIME };
  sc_core::sc_time  m_load_time    { SC_ZERO_TIME };
  sc_core::sc_time  m_start_time   { SC_ZERO_TIME };
  sc_core::sc_time  m_pulse_time   { SC_ZERO_TIME };
  sc_core::sc_time  m_resume_delay { SC_ZERO_TIME };
  sc_core::sc_time  m_timeout_time { SC_ZERO_TIME }; //< 0=>stopped
  bool              m_expired      { false }; //< indicates timer has expired
  bool              m_reload       { false }; // set to cause automatic reload of initial delay
  bool              m_continuous   { false };
  bool              m_count_up     { false };
  bool              m_overflowed   { false };
  bool              m_paused       { false };
  sc_core::sc_event m_timeout_event;
  sc_core::sc_event m_pulse_event;
};
#endif
