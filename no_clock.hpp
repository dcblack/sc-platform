#ifndef NONCLOCK_HPP
#define NONCLOCK_HPP

///////////////////////////////////////////////////////////////////////////////
// $License: Apache 2.0 $
//
// This file is licensed under the Apache License, Version 2.0 (the "License").
// You may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
//
// Description: See ABOUT_TIMER.md
// 
////////////////////////////////////////////////////////////////////////////////

#include "no_clock_if.hpp"
#include <systemc>
#include <map>

////////////////////////////////////////////////////////////////////////////////
//
//  #    # #######  ###  #      ###  #######  ###  #####   ####                          
//  #    #    #      #   #       #      #      #   #      #    #                         
//  #    #    #      #   #       #      #      #   #      #                              
//  #    #    #      #   #       #      #      #   #####   ####                          
//  #    #    #      #   #       #      #      #   #           #                         
//  #    #    #      #   #       #      #      #   #      #    #                         
//   ####     #     ###  #####  ###     #     ###  #####   ####                          
//
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
#if SYSTEMC_VERSION < 20140417
// Calculate a time remainder (missing from sc_time)
inline sc_core::sc_time operator % ( const sc_core::sc_time& lhs, const sc_core::sc_time& rhs )
{
  double lhd(lhs.to_seconds());
  double rhd(rhs.to_seconds());
  double rslt = lhd - rhd*int(lhd/rhd);
  return sc_core::sc_time(rslt,sc_core::SC_SEC);
}

#endif
//------------------------------------------------------------------------------
// For consistency
inline sc_core::sc_time operator % ( const sc_core::sc_time& lhs, const double& rhs )
{
  double lhd(lhs.to_seconds());
  double rslt = lhd - rhs*int(lhd/rhs);
  return sc_core::sc_time(rslt,sc_core::SC_SEC);
}

// For safety
inline
sc_core::sc_time sc_core_time_diff
( sc_core::sc_time& lhs
, sc_core::sc_time& rhs
, char* file
, int   lnum
)
{
  if (lhs >= rhs) return (lhs - rhs);
  static const char* MSGID="/Doulos/time_diff";
#ifdef INVERT_NEGATIVE
  sc_core::sc_report_handler::report
    ( sc_core::SC_WARNING
    , MSGID
    , "Negative time calculation returning the negative difference"
    , file, lnum);
  return (rhs - lhs);
#else
  sc_core::sc_report_handler::report
    ( sc_core::SC_WARNING
    , MSGID
    , "Negative time calculation returning SC_ZERO_TIME"
    , file, lnum);
  return sc_core::SC_ZERO_TIME;
#endif
}

#define sc_time_diff(lhs,rhs) sc_core_time_diff(lhs,rhs,__FILE__,__LINE__)

////////////////////////////////////////////////////////////////////////////////
//
//  #     #  ####           ####  #      ####   ####  #    #                      
//  ##    # #    #         #    # #     #    # #    # #   #                       
//  # #   # #    #         #      #     #    # #      #  #                        
//  #  #  # #    #         #      #     #    # #      ###                         
//  #   # # #    #         #      #     #    # #      #  #                        
//  #    ## #    #         #    # #     #    # #    # #   #                       
//  #     #  ####  #######  ####  #####  ####   ####  #    #                      
//
////////////////////////////////////////////////////////////////////////////////
#include "no_clock_if.hpp"
using get_time_t = sc_core::sc_time(*)(void);
struct no_clock
: sc_core::sc_object
, no_clock_if
{
  no_clock //< Constructor (enhanced)
  ( const char*             clock_instance
  , const sc_core::sc_time& tPERIOD
  , double                  duty
  , const sc_core::sc_time& tOFFSET
  , const sc_core::sc_time& tSAMPLE
  , const sc_core::sc_time& tSETEDGE
  , bool                    positive
  );
  no_clock //< Constructor (original)
  ( const char*             clock_instance
  , const sc_core::sc_time& tPERIOD
  , double                  duty     = 0.5
  , const sc_core::sc_time& tOFFSET  = sc_core::SC_ZERO_TIME
  , bool                    positive = true
  );

  // Use following to create clock and retrieve pointer
  static no_clock& global // Global clock accessor
  ( const std::string& clock_name
  , sc_core::sc_time   tPERIOD
  , double             duty     = 0.5
  , sc_core::sc_time   tOFFSET  = sc_core::SC_ZERO_TIME
  , sc_core::sc_time   tSAMPLE  = sc_core::SC_ZERO_TIME
  , sc_core::sc_time   tSETEDGE = sc_core::SC_ZERO_TIME
  , bool               positive = true
  );
  // Use following to retrieve reference
  static no_clock& global ( const std::string& clock_name);

  virtual ~no_clock(void); //< Destructor

  // Accessors
  void set_frequency          ( double           frequency );
  void set_period_time        ( sc_core::sc_time tPERIOD   );
  void set_offset_time        ( sc_core::sc_time tOFFSET   );
  void set_duty_cycle         ( double           duty      );
  void set_sample_time        ( sc_core::sc_time tSAMPLE   );
  void set_setedge_time       ( sc_core::sc_time tSETEDGE   );
  void set_time_shift         ( sc_core::sc_time tSHIFT    );
  const char*      clock_name ( void ) const;
  sc_core::sc_time period     ( Clock_count_t cycles = 1 ) const;
  double           duty       ( void ) const;
  double           frequency  ( void ) const;
  sc_core::sc_time time_shift ( void ) const { return m_tSHIFT; }
  // Special conveniences
  Clock_count_t    cycles ( void ) const; // Number of clock cycles since start
  Clock_count_t    cycles ( sc_core::sc_time t ) const;
  void             reset  ( void ); // Clears count & frequency change base (not yet implemented -- specification issues remain)
  Clock_count_t    frequency_changes ( void ) const { return m_freq_count; } // Number of times frequency was changed
  // Calculate the delay till... (use for temporal offset)...may return SC_ZERO_TIME if already on the edge
  sc_core::sc_time  until_posedge ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  until_negedge ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  until_anyedge ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  until_sample  ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  until_setedge ( Clock_count_t cycles = 0U ) const;
  // Calculate the delay till... (use for temporal offset)...never returns SC_ZERO_TIME
  sc_core::sc_time  next_posedge  ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  next_negedge  ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  next_anyedge  ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  next_sample   ( Clock_count_t cycles = 0U ) const;
  sc_core::sc_time  next_setedge  ( Clock_count_t cycles = 0U ) const;
  // Wait only if really necessary (for use in SC_THREAD)
  void wait         ( Clock_count_t cycles = 0U );
  void wait_posedge ( Clock_count_t cycles = 0U );
  void wait_negedge ( Clock_count_t cycles = 0U );
  void wait_anyedge ( Clock_count_t cycles = 0U );
  void wait_sample  ( Clock_count_t cycles = 0U );
  void wait_setedge ( Clock_count_t cycles = 0U );
  // Are we there? (use in SC_METHOD)
  bool at_posedge_time ( void ) const;
  bool posedge         ( void ) const { return at_posedge_time(); }
  bool at_negedge_time ( void ) const;
  bool negedge         ( void ) const { return at_negedge_time(); }
  bool at_anyedge_time ( void ) const;
  bool event           ( void ) const { return at_anyedge_time(); }
  bool at_sample_time  ( void ) const;
  bool at_setedge_time ( void ) const;
  // For compatibility if you really have/want to (with an extra feature)
  // - specify N > 0 to delay further out
  sc_core::sc_event& default_event       ( size_t events = 0 );
  sc_core::sc_event& posedge_event       ( size_t events = 0 );
  sc_core::sc_event& negedge_event       ( size_t events = 0 );
  sc_core::sc_event& sample_event        ( size_t events = 0 );
  sc_core::sc_event& setedge_event       ( size_t events = 0 );
  sc_core::sc_event& value_changed_event ( size_t events = 0 );
  bool      read                ( void ) const;

  virtual void write( const bool& ) { SC_REPORT_ERROR(MSGID,"write() not allowed on clock"); }
  virtual sc_core::sc_time delay( sc_core::sc_time tPERIOD, sc_core::sc_time tOFFSET, sc_core::sc_time tSHIFT) const;
  virtual Clock_count_t clocks( sc_core::sc_time tPERIOD, sc_core::sc_time tZERO, sc_core::sc_time tSHIFT) const;

private:
  // Don't allow copying
  no_clock(const no_clock& ) = delete; // Copy constructor
  no_clock(no_clock&& ) = delete; // Move constructor
  no_clock& operator= (const no_clock& ) = delete; // Copy assignment
  no_clock& operator= (no_clock&& ) = delete; // Move assignment
  // Report global clock statistics
  void end_of_simulation( void );
  // Internal data
  get_time_t          m_get_time;// callback that returns current time
  const char *        m_clock_name;// clock name
  sc_core::sc_time    m_tPERIOD; // clock period
  double              m_duty;    // clock duty cycle (fraction high)
  sc_core::sc_time    m_tOFFSET; // offset to first edge
  sc_core::sc_time    m_tPOSEDGE;// offset to positive rising edge (calculated)
  sc_core::sc_time    m_tNEGEDGE;// offset to negative falling edge (calculated)
  bool                m_posedge; // start on posedge
  sc_core::sc_time    m_tSAMPLE; // when to read (usually tOFFSET+tHOLD)
  sc_core::sc_time    m_tSETEDGE; // when to write (usually after tSAMPLE, but at least tPERIOD-tSETUP)
  sc_core::sc_event   m_anyedge_event;
  sc_core::sc_event   m_posedge_event;
  sc_core::sc_event   m_negedge_event;
  sc_core::sc_event   m_sample_event;
  sc_core::sc_event   m_setedge_event;
  sc_core::sc_time    m_frequency_set; // time when period was last changed
  Clock_count_t       m_freq_count{0ul};    // counts how many times frequency was changed
  Clock_count_t       m_base_count{0ul};    // cycles up to last frequency change
  sc_core::sc_time    m_tSHIFT{sc_core::SC_ZERO_TIME};  // temporal shift
  using clock_map_t = std::map<std::string,no_clock*>;
  static clock_map_t  s_global;
  static const char*  MSGID;
};

////////////////////////////////////////////////////////////////////////////////
//
//  ### #     # #     ### #     # #####                                           
//   #  ##    # #      #  ##    # #                                               
//   #  # #   # #      #  # #   # #                                               
//   #  #  #  # #      #  #  #  # #####                                           
//   #  #   # # #      #  #   # # #                                               
//   #  #    ## #      #  #    ## #                                               
//  ### #     # ##### ### #     # #####                                           
//
////////////////////////////////////////////////////////////////////////////////
// For efficiency

//------------------------------------------------------------------------------
inline void no_clock::set_time_shift (sc_core::sc_time tSHIFT) { m_tSHIFT = tSHIFT; }

//------------------------------------------------------------------------------
// Calculate the delay necessary to get to a particular time offset for a clock
// Example: 
//  wait(42,SC_NS); // from SC_ZERO_TIME
//  assert(delay(sc_time(10,SC_NS)) == sc_time(8,SC_NS));
//  assert(delay(sc_time(10,SC_NS),sc_time(3,SC_NS)) == sc_time(11,SC_NS));
//  assert(delay(sc_time(10,SC_NS),sc_time(1,SC_NS)) == sc_time(9,SC_NS));
//  Returns a value (0..tPERIOD-) + tOFFSET -- thus may be zero
inline sc_core::sc_time no_clock::delay 
( sc_core::sc_time tPERIOD // clock period
, sc_core::sc_time tOFFSET // beyond clock edge
, sc_core::sc_time tSHIFT  // temporal offset to assume
) const
{
  sc_core::sc_time tREMAINDER = (sc_core::sc_time_stamp() + tSHIFT) % tPERIOD;
  if      ( tREMAINDER == tOFFSET ) return sc_core::SC_ZERO_TIME;
  else if ( tREMAINDER <  tOFFSET ) return tOFFSET - tREMAINDER;
  else                              return tPERIOD + tOFFSET - tREMAINDER;
}

//------------------------------------------------------------------------------
// Compute the number of clocks since tZERO
inline Clock_count_t no_clock::clocks
( sc_core::sc_time tPERIOD
, sc_core::sc_time tZERO
, sc_core::sc_time tSHIFT
) const
{
  // TODO: Factor in frequency changes
  return (Clock_count_t)(( sc_core::sc_time_stamp() + tSHIFT - tZERO ) / tPERIOD);
}

//------------------------------------------------------------------------------
// Accessors
//------------------------------------------------------------------------------
inline const char*      no_clock::clock_name ( void ) const { return m_clock_name; }
inline sc_core::sc_time no_clock::period     ( Clock_count_t cycles ) const { return cycles*m_tPERIOD; }
inline double           no_clock::duty       ( void ) const { return m_duty; }
inline double           no_clock::frequency  ( void ) const { return sc_core::sc_time(1,sc_core::SC_SEC)/m_tPERIOD; }

//------------------------------------------------------------------------------
// Special conveniences
//------------------------------------------------------------------------------
inline Clock_count_t    no_clock::cycles ( void ) const // Number of clock cycles since start
{
  return m_base_count + clocks(m_tPERIOD,m_frequency_set,m_tSHIFT);
}

inline Clock_count_t    no_clock::cycles ( sc_core::sc_time t ) const // Number of clock cycles since start
{
  return static_cast<Clock_count_t>( t / m_tPERIOD );
}

// Calculate the delay till... (use for temporal offset)
inline sc_core::sc_time  no_clock::until_posedge ( Clock_count_t cycles ) const
{
  return cycles*m_tPERIOD+delay(m_tPERIOD,m_tPOSEDGE,m_tSHIFT);
}

inline sc_core::sc_time  no_clock::until_negedge ( Clock_count_t cycles ) const
{
  return cycles*m_tPERIOD+delay(m_tPERIOD,m_tNEGEDGE,m_tSHIFT);
}

inline sc_core::sc_time  no_clock::until_anyedge ( Clock_count_t cycles ) const
{
  return cycles*m_tPERIOD+(true==read()?until_negedge():until_posedge());
}

inline sc_core::sc_time  no_clock::until_sample  ( Clock_count_t cycles ) const
{
  return cycles*m_tPERIOD+delay(m_tPERIOD,m_tSAMPLE,m_tSHIFT);
}

inline sc_core::sc_time  no_clock::until_setedge ( Clock_count_t cycles ) const
{
  return cycles*m_tPERIOD+delay(m_tPERIOD,m_tSETEDGE,m_tSHIFT);
}

// Calculate the delay till next... (use for temporal offset) - never returns 0
inline sc_core::sc_time  no_clock::next_posedge ( Clock_count_t cycles ) const
{
  sc_core::sc_time t(delay(m_tPERIOD,m_tPOSEDGE,m_tSHIFT));
  return (cycles + (sc_core::SC_ZERO_TIME == t?1:0)) * m_tPERIOD + t;
}

inline sc_core::sc_time  no_clock::next_negedge ( Clock_count_t cycles ) const
{
  sc_core::sc_time t(delay(m_tPERIOD,m_tNEGEDGE,m_tSHIFT));
  return (cycles + (sc_core::SC_ZERO_TIME == t?1:0)) * m_tPERIOD + t;
}

inline sc_core::sc_time  no_clock::next_anyedge ( Clock_count_t cycles ) const
{
  return cycles*m_tPERIOD+(true==read()?next_negedge():next_posedge());
}

inline sc_core::sc_time  no_clock::next_sample  ( Clock_count_t cycles ) const
{
  sc_core::sc_time t(delay(m_tPERIOD,m_tSAMPLE,m_tSHIFT));
  return (cycles + (sc_core::SC_ZERO_TIME == t?1:0)) * m_tPERIOD + t;
}

inline sc_core::sc_time  no_clock::next_setedge ( Clock_count_t cycles ) const
{
  sc_core::sc_time t(delay(m_tPERIOD,m_tSETEDGE,m_tSHIFT));
  return (cycles + (sc_core::SC_ZERO_TIME == t?1:0)) * m_tPERIOD + t;
}

// Wait only if really necessary (for use in SC_THREAD) -- may be a NOP if cycles == 0
inline void no_clock::wait ( Clock_count_t cycles )
{
  if ( cycles > 0) sc_core::wait( period(cycles) );
}

inline void no_clock::wait_posedge ( Clock_count_t cycles )
{
  sc_core::sc_time t(until_posedge(cycles));
  if (sc_core::SC_ZERO_TIME != t) sc_core::wait(t);
}

inline void no_clock::wait_negedge ( Clock_count_t cycles )
{
  sc_core::sc_time t(until_negedge(cycles));
  if (sc_core::SC_ZERO_TIME != t) sc_core::wait(t);
}

inline void no_clock::wait_anyedge ( Clock_count_t cycles )
{
  sc_core::sc_time t(until_anyedge(cycles));
  if (sc_core::SC_ZERO_TIME != t) sc_core::wait(t);
}

inline void no_clock::wait_sample  ( Clock_count_t cycles )
{
  sc_core::sc_time t(until_sample(cycles));
  if (sc_core::SC_ZERO_TIME != t) sc_core::wait(t);
}

inline void no_clock::wait_setedge ( Clock_count_t cycles )
{
  sc_core::sc_time t(until_setedge(cycles));
  if (sc_core::SC_ZERO_TIME != t) sc_core::wait(t);
}

// Are we there? (use in SC_METHOD)
inline bool no_clock::at_posedge_time ( void ) const
{
  return until_posedge(0) == sc_core::SC_ZERO_TIME;
}

inline bool no_clock::at_negedge_time ( void ) const
{
  return until_negedge(0) == sc_core::SC_ZERO_TIME;
}

inline bool no_clock::at_anyedge_time ( void ) const
{
  return until_anyedge(0) == sc_core::SC_ZERO_TIME;
}

inline bool no_clock::at_sample_time  ( void ) const
{
  return until_sample(0) == sc_core::SC_ZERO_TIME;
}

inline bool no_clock::at_setedge_time ( void ) const
{
  return until_sample(0) == sc_core::SC_ZERO_TIME;
}

// For compatibility if you really have/want to
inline sc_core::sc_event& no_clock::default_event       ( size_t events )
{
  return value_changed_event(events);
}

inline sc_core::sc_event& no_clock::posedge_event       ( size_t events )
{
  wait_posedge(events);
  m_posedge_event.notify(sc_core::SC_ZERO_TIME);
  return m_posedge_event;
}

inline sc_core::sc_event& no_clock::negedge_event       ( size_t events )
{
  wait_negedge(events);
  m_negedge_event.notify(sc_core::SC_ZERO_TIME);
  return m_negedge_event;
}

inline sc_core::sc_event& no_clock::sample_event        ( size_t events )
{
  wait_sample(events);
  m_sample_event.notify(sc_core::SC_ZERO_TIME);
  return m_sample_event;
}

inline sc_core::sc_event& no_clock::setedge_event       ( size_t events )
{
  wait_setedge(events);
  m_setedge_event.notify(sc_core::SC_ZERO_TIME);
  return m_setedge_event;
}

inline sc_core::sc_event& no_clock::value_changed_event ( size_t events )
{
  wait_anyedge(events);
  m_anyedge_event.notify(sc_core::SC_ZERO_TIME);
  return m_anyedge_event;
}

inline bool               no_clock::read                ( void ) const
{
  return until_negedge()<until_posedge();
}

#endif

// TAF!
