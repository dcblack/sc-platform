#ifndef TIMER_BEH_HPP
#define TIMER_BEH_HPP

#include <systemc>
#include <string>

struct Timer
: sc_core::sc_module
{
  using sc_module_name = sc_core::sc_module_name;
  using sc_time        = sc_core::sc_time;
  using sc_event       = sc_core::sc_event;

  Timer( sc_module_name nm );   //< Constructor
  ~Timer( void );                        //< Destructor

  // Processes
  void trigger_thread( void );

  // Inline accessors
  void    set_load_delay  ( sc_time t )  { m_load_delay   = t;    }
  //id    set_start_time  ( sc_time t )  { m_start_time   = t;    }
  void    set_pulse_delay ( sc_time t )  { m_pulse_delay  = t;    }
  //id    set_trigger_time( sc_time t )  { m_trigger_time = t;    }
  sc_time get_load_delay  ( void ) const { return m_load_delay;   }
  sc_time get_start_time  ( void ) const { return m_start_time;   }
  sc_time get_pulse_delay ( void ) const { return m_pulse_delay;  }
  sc_time get_trigger_time( void ) const { return m_trigger_time; }
  void    set_triggered   ( bool v=true) { m_triggered  = v;      }
  void    clr_triggered   ( void )       { m_triggered  = false;  }
  void    set_reload      ( bool v=true) { m_reload     = v;      }
  void    set_continuous  ( bool v=true) { m_continuous = v;      }
  void    set_one_shot    ( void )       { m_continuous = false;  }
  bool    get_triggered   ( void ) const { return m_triggered;    }
  bool    get_reload      ( void ) const { return m_reload;       }
  bool    get_continuous  ( void ) const { return m_continuous;   }
  bool    is_paused       ( void ) const { return m_paused;       }

  // Fundamental behaviors of model
  sc_time         curr_time    ( sc_time delay = sc_core::SC_ZERO_TIME ) const;
  void            start        ( sc_time delay = sc_core::SC_ZERO_TIME );
  void            stop         ( sc_time delay = sc_core::SC_ZERO_TIME );
  void            pause        ( sc_time delay = sc_core::SC_ZERO_TIME );
  void            resume       ( sc_time delay = sc_core::SC_ZERO_TIME );
  const sc_event& trigger_event( void ) const
  {
    return m_trigger_event;  //< triggered when timer expires
  }
  const sc_event& pulse_event( void ) const
  {
    return m_pulse_event;  //< triggered when pulse time completes
  }

  // how much time remains until next trigger
  sc_time get_time_left( sc_time delay = sc_core::SC_ZERO_TIME ) const;
  sc_time get_time_since( sc_time delay = sc_core::SC_ZERO_TIME ) const;
  bool    is_running   ( sc_time delay = sc_core::SC_ZERO_TIME );

  // Indicate if trigger event was noted, but only do so once per trigger
  bool test_and_clear_triggered( void )
  {
    bool result = get_triggered();
    clr_triggered();
    return result;
  }
  std::string to_string( void );
  void reset( void );

private:
  // Terminology:
  //   _time implies absolute time (sc_time_stamp())
  //   _delay is relative time
  sc_time   m_load_delay   { sc_core::SC_ZERO_TIME }; //< trigger_time - start_time
  sc_time   m_start_time   { sc_core::SC_ZERO_TIME }; //< when timer started
  sc_time   m_pulse_delay  { sc_core::SC_ZERO_TIME }; //< nominally one clock period, but could change for PWM
  sc_time   m_resume_delay { sc_core::SC_ZERO_TIME }; //< time left on clock when paused
  sc_time   m_trigger_time { sc_core::SC_ZERO_TIME }; //< absolute time when timeout happens = start_time + load_delay // (0 => stopped)
  bool      m_triggered    { false }; //< indicates timer has triggered
  bool      m_reload       { false }; //< set to cause automatic retrigger computation on trigger
  bool      m_continuous   { false }; //< set to keep timer going after trigger
  bool      m_paused       { false }; //< indicates if paused
  sc_event  m_trigger_event;
  sc_event  m_pulse_event;
};
#endif /* TIMER_BEH_HPP */
