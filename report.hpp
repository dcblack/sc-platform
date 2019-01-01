#ifndef REPORT_HPP
#define REPORT_HPP
////////////////////////////////////////////////////////////////////////////////
//
//  #####  ##### #####   ####  #####  #######                                     
//  #    # #     #    # #    # #    #    #                                        
//  #    # #     #    # #    # #    #    #                                        
//  #####  ##### #####  #    # #####     #                                        
//  #  #   #     #      #    # #  #      #                                        
//  #   #  #     #      #    # #   #     #                                        
//  #    # ##### #       ####  #    #    #                                        
//
////////////////////////////////////////////////////////////////////////////////
// Improve reporting with macros, more overloads on `operator<<`,
// and other enhancements to `sc_report_handler`.
//
// See `ABOUT_REPORT.md` for more information.
#include <systemc>
#include <tlm>
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

// Use the following to (A) add more information in the event of failure, and
// and (B) control sc_assert behavior (i.e. not unconditional abort on failure).
#ifndef NDEBUG
#define ASSERT(expr,stream) do {\
  if(!(expr)) SC_REPORT_FATAL( "Assertion failed: ", # expr << " " << stream );\
} while (0)
#else
#define ASSERT(expr)
#endif

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
#define RULER(c) MESSAGE( std::string( 80, c ) << "\n" )

#define TODO(stream) REPORT( WARNING, "TODO: " << stream )
#define NOT_YET_IMPLEMENTED() REPORT( WARNING, __func__ << " is not yet implemented." )

std::string to_string( tlm::tlm_command command );
std::string to_string( uint8_t const * const data, uint32_t len );
std::string verbosity2str(const int& level);
template<typename T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& vec );

////////////////////////////////////////////////////////////////////////////////
// Implementation
template<typename T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& vec )
{
  static const int threshold{8};
  os << "{ ";
  size_t i = 0;

  for ( auto& v : vec ) {
    if ( i != 0 ) {
      os << ", ";
    }

    if ( i+1 == vec.size() and vec.size() > threshold ) {
      os << " ...";
    }

    if ( i < threshold or i+1 == vec.size() ) {
      os << i++ << ":" << v;
    }

  }

  os << " }";
  return os;
}

#endif
