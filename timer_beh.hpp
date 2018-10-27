#ifndef TIMER_BEH_HPP
#define TIMER_BEH_HPP

#include <systemc>

struct Timer
  : sc_core::sc_module {
  Timer( sc_core::sc_module_name nm );   //< Constructor
  ~Timer( void );                        //< Destructor

  // Processes
  void trigger_thread( void );

  // Inline accessors
  void set_trig_delay( sc_core::sc_time t ) { m_trig_delay = t; }
  void set_start_time( sc_core::sc_time t ) { m_start_time = t; }
  void set_pulse_time( sc_core::sc_time t ) { m_pulse_time = t; }
  void set_trigger_time( sc_core::sc_time t ) { m_trigger_time = t; }
  sc_core::sc_time get_trig_delay( void ) const { return m_trig_delay; }
  sc_core::sc_time get_start_time( void ) const { return m_start_time; }
  sc_core::sc_time get_pulse_time( void ) const { return m_pulse_time; }
  sc_core::sc_time get_trigger_time( void ) const { return m_trigger_time; }
  void set_triggered( bool v ) { m_triggered = v; }
  void set_reload( bool v ) { m_reload = v; }
  void set_continuous( bool v ) { m_continuous = v; }
  bool get_triggered( void ) const { return m_triggered; }
  bool get_reload( void ) const { return m_reload; }
  bool get_continuous( void ) const { return m_continuous; }
  bool is_paused( void ) const { return m_paused; }

  // Fundamental behaviors of model
  sc_core::sc_time curr_time( sc_core::sc_time delay = sc_core::SC_ZERO_TIME ) const;
  void start ( sc_core::sc_time delay = sc_core::SC_ZERO_TIME );
  void stop  ( sc_core::sc_time delay = sc_core::SC_ZERO_TIME );
  void pause ( sc_core::sc_time delay = sc_core::SC_ZERO_TIME );
  void resume( sc_core::sc_time delay = sc_core::SC_ZERO_TIME );
  const sc_core::sc_event& trigger_event( void ) const
  {
    return m_trigger_event;  //< triggered when timer expires
  }

  sc_core::sc_time get_time_left( sc_core::sc_time delay = sc_core::SC_ZERO_TIME ) const;  //< how much time remains
  bool is_running( sc_core::sc_time delay = sc_core::SC_ZERO_TIME );

  bool triggered( void ) //< indicates if trigger event was noted, but only does so once per trigger
  {
    bool result = m_triggered;
    m_triggered = false;
    return result;
  }

private:
  sc_core::sc_time   m_trig_delay   { sc_core::SC_ZERO_TIME }; // trigger_time - start_time
  sc_core::sc_time   m_start_time   { sc_core::SC_ZERO_TIME }; // when timer started
  sc_core::sc_time   m_pulse_time   { sc_core::SC_ZERO_TIME }; // nominally one clock period, but could change for PWM
  sc_core::sc_time   m_resume_delay { sc_core::SC_ZERO_TIME };
  sc_core::sc_time   m_trigger_time { sc_core::SC_ZERO_TIME }; //< start_time + trig_delay // (0 => stopped)
  bool               m_triggered    { false }; //< indicates timer has triggered
  bool               m_reload       { false }; // set to cause automatic retrigger computation on trigger
  bool               m_continuous   { false }; // set to keep timer going after trigger
  bool               m_paused       { false };
  sc_core::sc_event  m_trigger_event;
  static const char* MSGID;
};
#endif /* TIMER_BEH_HPP */
