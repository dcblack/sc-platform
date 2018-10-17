#include "no_clock.hpp"

///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// This is an implemenation of the no_clock component used
// to simplify high-level modeling of clocks without incurring
// the context switching overhead of a real clock.

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

#include <systemc>
#include <string>
using namespace sc_core;
using namespace std;

const char*  no_clock::MSGID = "/Doulos/no_clock";
no_clock::clock_map_t no_clock::s_global;

no_clock& no_clock::global // Global clock accessor
( const char* clock_name
, sc_time     tPERIOD
, double      duty
, sc_time     tOFFSET
, sc_time     tSAMPLE
, sc_time     tSETEDGE
, bool        positive
)
{
  string message;
  if (s_global.count(clock_name) != 0) {
    message = "Attempt to create global clock with existing name '";
    message += clock_name;
    message += "'";
    SC_REPORT_FATAL(MSGID,message.c_str());
  }//endif
  message = "Creating new global clock '";
  message += clock_name;
  message += "'";
  SC_REPORT_INFO(MSGID,message.c_str());
  no_clock* clock_ptr = new no_clock
  ( clock_name
  , tPERIOD
  , duty
  , tOFFSET
  , tSAMPLE
  , tSETEDGE
  , positive
  );
  s_global.insert(make_pair(clock_name,clock_ptr));
  return *clock_ptr;
}

no_clock& no_clock::global
( const char* clock_name
)
{
  auto clock_it(s_global.find(clock_name));
  if (clock_it == s_global.end()) {
    string message("Missing definition for global clock '");
    message += clock_name;
    message += "'";
    SC_REPORT_FATAL("/Doulos/no_clock/global",message.c_str());
  }//endif
  return *(clock_it->second);
}

//------------------------------------------------------------------------------
no_clock::no_clock //< Constructor
( const char*    clock_instance
, const sc_time& tPERIOD
, double         duty
, const sc_time& tOFFSET
, const sc_time& tSAMPLE
, const sc_time& tSETEDGE
, bool           positive
)
: m_clock_name(clock_instance)
, m_tPERIOD(tPERIOD)
, m_duty(duty)
, m_tOFFSET(tOFFSET)
, m_tPOSEDGE((positive)?(tOFFSET):(tOFFSET+duty*m_tPERIOD))
, m_tNEGEDGE((positive)?(tOFFSET+duty*m_tPERIOD):(tOFFSET))
, m_posedge(positive)
, m_tSAMPLE(tSAMPLE)
, m_tSETEDGE(tSETEDGE)
, m_frequency_set(sc_time_stamp())
{
  if (tPERIOD <= SC_ZERO_TIME) {
    SC_REPORT_FATAL(MSGID,"Clocks must have a positive non-zero period.");
  }//endif
  if (duty <= 0.0 or 1.0 <= duty) {
    SC_REPORT_FATAL(MSGID,"Duty cycle must be greater than 0.0 and less than 1.0!");
  }//endif
  if (tOFFSET >= tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tOFFSET must be less than period.");
  }//endif
  if (tSAMPLE < SC_ZERO_TIME or tSAMPLE >= tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tSAMPLE must be non-negative and less than period.");
  }//endif
  if (tSETEDGE < SC_ZERO_TIME or tSETEDGE >= tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tSETEDGE must be non-negative and less than period.");
  }//endif
}

//------------------------------------------------------------------------------
no_clock::no_clock //< Constructor
( const char*    clock_instance
, const sc_time& tPERIOD
, double         duty
, const sc_time& tOFFSET
, bool           positive
)
: m_clock_name(clock_instance)
, m_tPERIOD(tPERIOD)
, m_duty(duty)
, m_tOFFSET(tOFFSET)
, m_posedge(positive)
, m_tSAMPLE(positive?(tOFFSET):(tOFFSET+(1-duty)*tPERIOD))
, m_tSETEDGE(positive?(tOFFSET+duty*tPERIOD):(tOFFSET))
, m_frequency_set(sc_time_stamp())
{
  if (tPERIOD <= SC_ZERO_TIME) {
    SC_REPORT_FATAL(MSGID,"Clocks must have a positive non-zero period.");
  }//endif
  if (duty <= 0.0 or 1.0 <= duty) {
    SC_REPORT_FATAL(MSGID,"Duty cycle must be greater than 0.0 and less than 1.0!");
  }//endif
  if (tOFFSET >= tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tOFFSET must be less than period.");
  }//endif
  if (m_tSAMPLE < SC_ZERO_TIME or m_tSAMPLE >= tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tSAMPLE must be non-negative and less than period.");
  }//endif
  if (m_tSETEDGE < SC_ZERO_TIME or m_tSETEDGE >= tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tSETEDGE must be non-negative and less than period.");
  }//endif
}

no_clock::~no_clock(void) //< Destructor
{
}

void no_clock::set_frequency
( double frequency )
{
  if (frequency <= 0.0) {
    SC_REPORT_FATAL(MSGID,"Clocks must have a positive non-zero frequency.");
  }//endif
  ++m_freq_count;
  m_base_count += cycles();
  m_tPERIOD = sc_time(1.0/frequency, SC_SEC); 
  m_frequency_set = sc_time_stamp();
}

//------------------------------------------------------------------------------
void no_clock::set_period_time
( sc_time tPERIOD )
{
  if (tPERIOD <= SC_ZERO_TIME) {
    SC_REPORT_FATAL(MSGID,"Clocks must have a positive non-zero period.");
  }//endif
  ++m_freq_count;
  m_base_count += cycles();
  m_tPERIOD = tPERIOD;
  m_frequency_set = sc_time_stamp();
}

//------------------------------------------------------------------------------
void no_clock::set_offset_time
( sc_time tOFFSET )
{
  m_tOFFSET = tOFFSET;
}

//------------------------------------------------------------------------------
void no_clock::set_duty_cycle        
( double duty )
{
  if (duty <= 0.0 or 1.0 <= duty) {
    SC_REPORT_FATAL(MSGID,"Duty cycle must be greater than 0.0 and less than 1.0!");
  }//endif
  m_duty = duty;
  m_tPOSEDGE = (m_posedge)?(m_tOFFSET):(m_tOFFSET+duty*m_tPERIOD);
  m_tNEGEDGE = (m_posedge)?(m_tOFFSET+duty*m_tPERIOD):(m_tOFFSET);
}

//------------------------------------------------------------------------------
void no_clock::set_sample_time       
( sc_time tSAMPLE )
{
  if (tSAMPLE < SC_ZERO_TIME or tSAMPLE >= m_tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tSAMPLE must be greater than SC_ZERO_TIME and less than tPERIOD.");
  }//endif
  m_tSAMPLE = tSAMPLE; 
}

//------------------------------------------------------------------------------
void no_clock::set_setedge_time       
( sc_time tSETEDGE )
{
  if (tSETEDGE < SC_ZERO_TIME or tSETEDGE >= m_tPERIOD) {
    SC_REPORT_FATAL(MSGID,"tSETEDGE must be greater than SC_ZERO_TIME and less than tPERIOD.");
  }//endif
  m_tSETEDGE = tSETEDGE; 
}

void no_clock::reset //< Clears count & frequency change base
( void )
{
  SC_REPORT_FATAL(MSGID,"Not yet implemented"); // {:TODO:}
}

// TAF!
