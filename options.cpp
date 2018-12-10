////////////////////////////////////////////////////////////////////////////////
//
//   ####  #####  ####### ###  ####  #     #  ####                                
//  #    # #    #    #     #  #    # ##    # #    #                               
//  #    # #    #    #     #  #    # # #   # #                                    
//  #    # #####     #     #  #    # #  #  #  ####                                
//  #    # #         #     #  #    # #   # #      #                               
//  #    # #         #     #  #    # #    ## #    #                               
//   ####  #         #    ###  ####  #     #  ####                                
//
////////////////////////////////////////////////////////////////////////////////
#include "options.hpp"
#include "common.hpp"
#include "report.hpp"
#include <string>
using namespace sc_core;
using namespace std;
namespace {
  const char * const MSGID{ "/Doulos/Example/Options" };
}

Options* const Options::instance( void )
{
  static Options options{};
  return &options;
}

Options::Options( void )
{
  m_interrupt.remove();
  // Establish defaults
  m_configuration = Interconnect::NORTH_SOUTH;

  // Pars command-line
  for ( int iArg = 1; iArg < sc_argc(); ++iArg )
  {
    std::string arg( sc_argv()[iArg] );

    //--------------------------------------------------------------------------
    // Brief help
    if ( arg == "-help" or arg == "--help" or arg == "-h" ) {
      MESSAGE( "\n" );
      RULER( '-' );
      MESSAGE( "\n"
               << "Syntax:\n"
               << "\n"
               << "  " << string( sc_argv()[0] ) << " OPTIONS\n"
               << "\n"
               << "Options:\n"
               << "\n"
               << "  -help        \n"
               << "  -hyper       \n"
               << "  -debug+1     \n"
               << "  -debug       \n"
               << "  -full        \n"
               << "  -high        \n"
               << "  -medium      \n"
               << "  -low         \n"
               << "  -none        \n"
               << "  -AT          \n"
               << "  -LT          \n"
               << "  -error-at-target\n"
               << "\n"
             );
      RULER( '-' );
      MEND( ALWAYS );
    }

    //--------------------------------------------------------------------------
    // Information message verbosity
    else if ( arg == "-hyper" ) {
      sc_report_handler::set_verbosity_level( SC_HYPER );
      m_interrupt.install();
    }
    else if ( arg == "-debug+1" ) {
      sc_report_handler::set_verbosity_level( SC_DEBUG + 1 );
      m_interrupt.install();
    }
    else if ( arg == "-debug" ) {
      sc_report_handler::set_verbosity_level( SC_DEBUG );
      m_interrupt.install();
    }
    else if ( arg == "-full" ) {
      sc_report_handler::set_verbosity_level( SC_FULL );
    }
    else if ( arg == "-high" ) {
      sc_report_handler::set_verbosity_level( SC_HIGH );
    }
    else if ( arg == "-medium" ) {
      sc_report_handler::set_verbosity_level( SC_MEDIUM );
    }
    else if ( arg == "-low" ) {
      sc_report_handler::set_verbosity_level( SC_LOW );
    }
    else if ( arg == "-none" ) {
      sc_report_handler::set_verbosity_level( SC_NONE );
    }

    //--------------------------------------------------------------------------
    // Coding style for initiators
    else if ( arg == "-AT" ) {
      g_coding_style = Style::AT;
    }
    else if ( arg == "-LT" ) {
      g_coding_style = Style::LT;

    }
    //--------------------------------------------------------------------------
    // Display error messages at point of detection rather than returning
    // an error response. Helpful during debug.
    else if ( arg == "-error-at-target" or arg == "-eat" ) {
      g_error_at_target = true;
    }

    //--------------------------------------------------------------------------
    // Configuration of top
    else if ( arg == "-cfg" ) {
      if( iArg+1 >= sc_argc() ) {
        REPORT( ERROR, "Missing required argument for " << arg << " option." );
      }
      arg = sc_argv()[++iArg];
      if ( arg == "trivial" ) {
        m_configuration = Interconnect::TRIVIAL;
      }
      else if ( arg == "memory" ) {
        m_configuration = Interconnect::MEMORY;
      }
      else if ( arg == "timer" ) {
        m_configuration = Interconnect::TIMER;
      }
      else if ( arg == "ns" ) {
        m_configuration = Interconnect::NORTH_SOUTH;
      }
    }

  }//endforeach arg

  sc_report_handler::set_actions( SC_ERROR, SC_DISPLAY | SC_LOG );
  INFO( ALWAYS, "Verbosity is "
        << verbosity2str( sc_report_handler::get_verbosity_level() )
        << "." );

  if ( g_error_at_target ) {
    INFO( ALWAYS, "Reporting errors at target." );
  }

  if( m_test_set.empty() ) m_test_set.insert(Test::TRIVIAL);
  INFO( ALWAYS, "Constructed options" );
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
