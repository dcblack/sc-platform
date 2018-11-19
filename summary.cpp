#include "summary.hpp"
#define REQUIRES_CPP 14
#include "require_cxx_version.hpp"
#include "report.hpp"
#include "wallclock.hpp"
#include <string>
using namespace std;
using namespace sc_core;
namespace {
  const char * const MSGID { "/Doulos/Example/summary" };
}

//------------------------------------------------------------------------------
// Static allocations
double       Summary::s_elaboration_time = -1.0;
double       Summary::s_starting_time    = -1.0;
double       Summary::s_finished_time    = -1.0;
unsigned int Summary::s_errors           = 0;
unsigned int Summary::s_warnings         = 0;

//------------------------------------------------------------------------------
void Summary::starting_elaboration( void )
{
  s_elaboration_time = get_cpu_time();
}

//------------------------------------------------------------------------------
void Summary::starting_simulation( void )
{
  s_starting_time = get_cpu_time();
}

//------------------------------------------------------------------------------
void Summary::finished_simulation( void )
{
  s_finished_time = get_cpu_time();
}

//------------------------------------------------------------------------------
int Summary::errors( void )
{
  return sc_report_handler::get_count( SC_ERROR )
         + s_errors
         + sc_report_handler::get_count( SC_FATAL );
}

//------------------------------------------------------------------------------
// Summarize results
int Summary::report( void )
{
  {
    bool warn{ false };
    MESSAGE( "\n" );
    RULER( '#' );
    MESSAGE( "Compilation information for " << sc_argv()[0] << ":\n" );
    MESSAGE( "  C++ std version: " << __cplusplus << " (" << CPP_VERSION << ")" << "\n" );
    MESSAGE( "  SystemC version: " << SYSTEMC_VERSION << "\n" );
    MESSAGE( "  TLM     version: " << TLM_VERSION << "\n" );
    #ifdef BOOST_LIB_VERSION
    MESSAGE( "  BOOST   version: " << BOOST_LIB_VERSION << "\n" );
    #endif
    #if defined(__clang__)
    MESSAGE( "  Compiled  using: Clang/LLVM version " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << "\n" );
    #elif defined(__ICC) || defined(__INTEL_COMPILER)
    MESSAGE( "  Compiled  using: Intel ICC/ICPC version " << __ICC << "\n" );
    #elif defined(__GNUC__) || defined(__GNUG__)
    MESSAGE( "  Compiled  using: GNU GCC/G++ version " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n" );
    #elif defined(__HP_cc) || defined(__HP_aCC)
    MESSAGE( "  Compiled  using: Hewlett-Packard C/aC++ version " << __HP_cc << "\n" );
    #elif defined(__IBMC__) || defined(__IBMCPP__)
    MESSAGE( "  Compiled  using: IBM XL C/C++ version " << __IBMCPP__ << "\n" );
    #elif defined(_MSC_VER)
    MESSAGE( "  Compiled  using: Microsoft Visual Studio version " << _MSC_FULL_VER << "\n" );
    #elif defined(__PGI)
    MESSAGE( "  Compiled  using: Portland Group PGCC/PGCPP version " << __PGIC__ << "." << __PGIC_MINOR__ << "." << __PGIC_PATCHLEVEL__ << "\n" );
    #elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    MESSAGE( "  Compiled  using: Oracle Solaris Studio version " << std::hex << __SUNPRO_CC << "\n" );
    #else
    MESSAGE( "  Unknown compiler?" << "\n" );
    ++s_warnings;
    warn = true;
    #endif
    MEND( ALWAYS );
    if( warn ) {
      REPORT( WARNING, " Please inform Doulos of your compiler and how to check for its existence." );
    }
  }
  string kind = "Simulation";

  if ( s_starting_time < 0.0 ) {
    kind = "Elaboration";
    s_starting_time = s_finished_time = get_cpu_time();
  }

  if ( s_finished_time < 0.0 ) {
    s_finished_time = get_cpu_time();
  }

  MESSAGE( "\n" );
  RULER( '-' );
  MESSAGE( "Summary for " << sc_argv()[0] << ":\n" );
  MESSAGE( "  CPU elaboration time " << setprecision( 4 ) << ( s_starting_time - s_elaboration_time ) << " sec\n" );
  MESSAGE( "  CPU simulation  time " << setprecision( 4 ) << ( s_finished_time - s_starting_time    ) << " sec\n" );
  MESSAGE( "  " << setw( 2 ) << sc_report_handler::get_count( SC_WARNING )          << " warnings" << "\n" );
  MESSAGE( "  " << setw( 2 ) << sc_report_handler::get_count( SC_ERROR ) + s_errors << " errors"   << "\n" );
  MESSAGE( "  " << setw( 2 ) << sc_report_handler::get_count( SC_FATAL )            << " fatals"   << "\n" );
  MESSAGE( "\n" );
  RULER( '#' );
  MESSAGE( kind << " " << ( errors() ? "FAILED" : "PASSED" ) );
  MEND( ALWAYS );
  return ( errors() ? 1 : 0 );
}
