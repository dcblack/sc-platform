// Improve reporting providing two macros, REPORT and INFO, that allow
// for streaming syntax like cout.
//
// Syntax:
//   REPORT(message_type, message_stream);
//   INFO(verbosity_level, message_stream);
//   MESSAGE(message_stream);
//   MEND(verbosity_level);
//   RULER(char);
//   TODO(message_stream);
//   NOT_YET_IMPLEMENTED();
//
//   message types are: FATAL, ERROR, WARNING, and INFO
//   verbosity levels are: ALWAYS, LOW, MEDIUM, HIGH, DEBUG
//
//   MESSAGE macro doesn't output anything, but rather builds up
//   a message to be emitted by REPORT, INFO or MEND.
//
// Assumes you define in every implementation file (i.e. .cpp):
//
//   #include "report.h"
//   namespace { static const char* MSGID{ "/Company/Group/Project/Module" }; }
//
// For header files leave off the name space and put in the function
// or define a class member (non-static). Must not allow to escape
// the header. So a #define is inappropriate.
//
// Examples:
#ifdef EXAMPLES
     #include "report.h"
     namespace { static const char* MSGID{ "/Doulos/Example/Report" }; }
     REPORT(ERROR,"Data " << data << " doesn't match expected " << expected);
     INFO(DEBUG,"Packet contains " << packet);
     TODO("Fix report handler to remove blank line after REPORT_INFO");
     NOT_YET_IMPLEMENTED();
     MESSAGE( "Map contents:\n" );
     for( const auto& v : my_map ) {
       MESSAGE( "  " << v.first << ": " << v.second << "\n" );
     }
     MEND( HIGH ); // or REPORT( WARNING, "" );
#endif
//

#ifndef REPORT_H
#define REPORT_H
#include <systemc>
#include <sstream>
#include <iomanip>
extern std::ostringstream mout;
#define HEX std::hex << std::showbase
#define DEC std::dec << std::noshowbase << std::setfill(' ')

#define REPORT(type,stream)                      \
do {                                             \
  mout << DEC << stream << std::ends;            \
  SC_REPORT_##type( MSGID, mout.str().c_str() ); \
  mout.str( "" );                                \
} while (0)

#define SC_ALWAYS SC_NONE
#define SC_NEVER  16*KB
#define SC_HYPER  1024
#define DEVID (std::string("(")+name()+")").c_str()
#define NOINFO(level,stream)
#define INFO(level,stream)                                                          \
do {                                                                                \
  if( sc_core::sc_report_handler::get_verbosity_level() >= (sc_core::SC_##level) ) {\
    mout << DEC << stream << std::ends;                                             \
    if( (sc_core::SC_##level) > sc_core::SC_DEBUG ) {                               \
      std::string id{"DEBUG("};                                                     \
      id+=__FILE__ ; id+=":"; id+=std::to_string(__LINE__)+")";                     \
      size_t p0=id.find("/"),p1=id.find_last_of("/");                               \
      if(p1!=std::string::npos) id.erase(p0,p1-p0+1);                               \
      SC_REPORT_INFO_VERB( id.c_str(), mout.str().c_str(), (sc_core::SC_##level) ); \
    } else                                                                          \
    SC_REPORT_INFO_VERB( MSGID, mout.str().c_str(), (sc_core::SC_##level) );        \
    mout.str( "" );                                                                 \
  }                                                                                 \
} while (0)

#define MESSAGE(stream) do { mout << stream; } while(0)
#define MEND(level) do {                                                            \
  if( sc_core::sc_report_handler::get_verbosity_level() >= (sc_core::SC_##level) ) {\
    mout << std::ends;                                                              \
    SC_REPORT_INFO_VERB( MSGID, mout.str().c_str(), (sc_core::SC_##level));         \
    mout.str( "" );                                                                 \
  }                                                                                 \
} while (0)
#define RULER(c) MESSAGE( string( 80, c ) << "\n" )

#define TODO(stream) REPORT( WARNING, "TODO: " << stream )
#define NOT_YET_IMPLEMENTED() REPORT( WARNING, __func__ << " is not yet implemented." )

#endif
