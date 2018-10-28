//BEGIN top.cpp (systemc)
// -*- C++ -*- vim600:syntax=cpp:sw=2:tw=78:fmr=<<<,>>>
// COMMENT BLOCK <<<///////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
// 
// DESCRIPTION
//! \brief Top level connectivity for platform.
//
// This is the top-level connectivity implementation. See
// header for high-level description.
//
////////////////////////////////////////////////////////////////////////////>>>
#include "no_clock.hpp"
#include "top.hpp"
#include "report.hpp"
#include "common.hpp"
#include "cpu.hpp"
#include "bus.hpp"
#include "memory.hpp"
#include "memory_map.hpp"
#include "timer.hpp"
using namespace sc_core;
using namespace std;
namespace {
  const char* const MSGID="/Doulos/Example/Platform";
  const char* const RCSID="$Id$";
}

///////////////////////////////////////////////////////////////////////////////
// Constructor
Top_module::Top_module( sc_module_name instance_name )
  : pImpl{ std::make_unique<Impl>() }
{
  for ( int i = 1; i < sc_core::sc_argc(); ++i ) {
    std::string arg( sc_core::sc_argv()[i] );

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

      //--------------------------------------------------------------------------
      // Information message verbosity
    }
    else if ( arg == "-hyper" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_HYPER );
    }
    else if ( arg == "-debug+1" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_DEBUG + 1 );
    }
    else if ( arg == "-debug" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_DEBUG );
    }
    else if ( arg == "-full" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_FULL );
    }
    else if ( arg == "-high" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_HIGH );
    }
    else if ( arg == "-medium" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_MEDIUM );
    }
    else if ( arg == "-low" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_LOW );
    }
    else if ( arg == "-none" ) {
      sc_core::sc_report_handler::set_verbosity_level( SC_NONE );

      //--------------------------------------------------------------------------
      // Coding style for initiators
    }
    else if ( arg == "-AT" ) {
      g_coding_style = Style::AT;
    }
    else if ( arg == "-LT" ) {
      g_coding_style = Style::LT;

      //--------------------------------------------------------------------------
      // Display error messages at point of detection rather than returning
      // an error response. Helpful during debug.
    }
    else if ( arg == "-error-at-target" or arg == "-eat" ) {
      g_error_at_target = true;
    }
  }

  sc_report_handler::set_actions( SC_ERROR, SC_DISPLAY | SC_LOG );
  INFO( ALWAYS, "Verbosity is "
        << verbosity2str( sc_report_handler::get_verbosity_level() )
        << "." );

  if ( g_error_at_target ) {
    INFO( ALWAYS, "Reporting errors at target." );
  }
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
Top_module::~Top_module( void )
{
  // Nothing to do :)
}

struct Top_module::Impl {
  // Clock
  no_clock& clk { no_clock::global( "system_clock", 100_MHz ) };

  // Modules
  std::unique_ptr<Cpu_module   > cpu;
  std::unique_ptr<Bus_module   > nth;
  std::unique_ptr<Memory_module> rom;
  std::unique_ptr<Memory_module> ram;
  std::unique_ptr<Timer_module>  tmr;

  // Constructor
  Impl( void )
  {
    config();
    switch ( m_configuration ) { // Fall-thru intentional
      case Configuration::TIMER:
        tmr = std::make_unique<Timer_module> ( "tmr" , 2, TMR_BASE, 1, 2, 2 );
      case Configuration::MEMORY:
        rom = std::make_unique<Memory_module>( "rom" , ROM_DEPTH, ROM_BASE,  Access::RO, 16, 32, DMI::enabled );
        nth = std::make_unique<Bus_module>   ( "nth" );
      case Configuration::TRIVIAL:
        ram = std::make_unique<Memory_module>( "ram" , RAM_DEPTH, RAM_BASE,  Access::RW, 16,  8, DMI::enabled );
        cpu = std::make_unique<Cpu_module>   ( "cpu" );
      default:
        break;
    }

    // Connectivity
    switch ( m_configuration ) {
      case Configuration::TRIVIAL:
        cpu->init_socket.bind( ram->targ_socket );
        break;

      case Configuration::MEMORY:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        break;

      case Configuration::TIMER:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        nth->init_socket.bind( tmr->targ_socket );
        tmr->intrq_port.bind ( cpu->intrq_xport );
        break;

      default:
        REPORT( FATAL, "Missing configuration." );
        break;
    }
  }

  ~Impl( void ) = default;

  // Helper methods
  void config( void )
  {
    // Establish default
    m_configuration = Configuration::TIMER;

    for ( int i = 1; i < sc_argc(); ++i ) {
      string arg{sc_argv()[i]};

      if ( arg == "-trivial" ) {
        m_configuration = Configuration::TRIVIAL;
      }
      else if ( arg == "-memory" ) {
        m_configuration = Configuration::MEMORY;
      }
      else if ( arg == "-timer" ) {
        m_configuration = Configuration::TIMER;
      }
    }
  }

  // Attributes
  enum class Configuration {
    DEFAULT
    , MEMORY
    , TIMER
    , PIC
    , TRIVIAL
  }
  m_configuration;
};

void Top_module::end_of_elaboration( void )
{
  MESSAGE( "Design complete" );
  MEND( ALWAYS );
  // TODO: Netlist
}

void Top_module::start_of_simulation( void )
{
  MESSAGE( "\n" );
  RULER('!');
  MESSAGE( "Clock period is " << pImpl->clk.period() );
  MEND( ALWAYS );
}

void Top_module::end_of_simulation( void )
{
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
