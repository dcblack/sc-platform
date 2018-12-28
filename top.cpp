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
#include "summary.hpp"
#include "common.hpp"
#include "netlist.hpp"
#include "cpu.hpp"
#include "options.hpp"
#include "bus.hpp"
#include "memory.hpp"
#include "memory_map.hpp"
#include "timer.hpp"
#include "pic.hpp"
#include "gpio.hpp"
#include "stub.hpp"
#include <set>
using namespace sc_core;
using namespace std;
namespace {
  const char* const MSGID="/Doulos/Example/Platform/top";
}

///////////////////////////////////////////////////////////////////////////////
// Constructor
Top_module::Top_module( sc_module_name instance_name )
  : pImpl{ std::make_unique<Impl>() }
{
  // Nothing to do
}

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
Top_module::~Top_module( void )
{
  // Nothing to do
}

///////////////////////////////////////////////////////////////////////////////
// Private implementation
struct Top_module::Impl
{

  // Clock
  no_clock& clk { no_clock::global( "system_clock", 100_MHz ) };

  // Modules
  std::unique_ptr< Cpu_module    > cpu;
  std::unique_ptr< Bus_module    > nth;
  std::unique_ptr< Bus_module    > sth;
  std::unique_ptr< Memory_module > rom;
  std::unique_ptr< Memory_module > ram;
  std::unique_ptr< Memory_module > ddr;
  std::unique_ptr< Gpio_module   > gio;
  std::unique_ptr< Timer_module  > tmr;
  std::unique_ptr< Pic_module    > pic;
  std::unique_ptr< Stub_module   > dma;

  // Constructor
  Impl( void )
  : options( Options::instance() )
  {
    // Instantiation
    switch ( options->get_configuration() ) { // Fall-thru intentional
      case Interconnect::DMA:
        dma = std::make_unique<Stub_module>  ( "dma" );
      case Interconnect::GPIO:
      case Interconnect::PIC:
        // fall thru
      case Interconnect::NORTH_SOUTH:
        sth = std::make_unique<Bus_module>   ( "sth" );
        ddr = std::make_unique<Memory_module>( "ddr" , 1*KB, Access::RW, 16,  8, DMI::enabled );
        // fall thru
      case Interconnect::TIMER:
        tmr = std::make_unique<Timer_module> ( "tmr" , 2, 1, 2, 2 );
        gio = std::make_unique<Gpio_module>  ( "gio" );
        pic = std::make_unique<Pic_module>   ( "pic" );
        // fall thru
      case Interconnect::MEMORY:
        rom = std::make_unique<Memory_module>( "rom" , 1*KB, Access::RO, 16, 32, DMI::enabled );
        nth = std::make_unique<Bus_module>   ( "nth" );
        // fall thru
      case Interconnect::TRIVIAL:
        ram = std::make_unique<Memory_module>( "ram" , 1*KB, Access::RW, 16,  8, DMI::enabled );
        cpu = std::make_unique<Cpu_module>   ( "cpu" );
        break;
      default:
        REPORT( FATAL, "Failed to create any modules!?" );
    }

    // Connectivity
    switch ( options->get_configuration() ) {
      case Interconnect::TRIVIAL:
        cpu->init_socket.bind( ram->targ_socket );
        break;

      case Interconnect::MEMORY:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        break;

      case Interconnect::TIMER:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        nth->init_socket.bind( gio->targ_socket );
        nth->init_socket.bind( tmr->targ_socket );
        nth->init_socket.bind( pic->targ_socket );
        // Interrupts
        tmr->intrq_port.bind ( cpu->intrq_xport );
        break;

      case Interconnect::NORTH_SOUTH:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        nth->init_socket.bind( ddr->targ_socket );
        nth->init_socket.bind( sth->targ_socket );
        sth->init_socket.bind( gio->targ_socket );
        sth->init_socket.bind( tmr->targ_socket );
        sth->init_socket.bind( pic->targ_socket );
        // Interrupts
        tmr->intrq_port.bind ( cpu->intrq_xport );
        break;

      case Interconnect::PIC:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        nth->init_socket.bind( ddr->targ_socket );
        nth->init_socket.bind( sth->targ_socket );
        sth->init_socket.bind( gio->targ_socket );
        sth->init_socket.bind( tmr->targ_socket );
        sth->init_socket.bind( pic->targ_socket );
        // Interrupts
        tmr->intrq_port.bind    ( pic->irq_recv_xport );
        pic->irq_send_port.bind ( cpu->intrq_xport    );
        break;

      case Interconnect::GPIO:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        nth->init_socket.bind( ddr->targ_socket );
        nth->init_socket.bind( sth->targ_socket );
        sth->init_socket.bind( gio->targ_socket );
        sth->init_socket.bind( tmr->targ_socket );
        sth->init_socket.bind( pic->targ_socket );
        // Interrupts
        tmr->intrq_port.bind    ( pic->irq_recv_xport );
        gio->intrq_port.bind    ( pic->irq_recv_xport );
        pic->irq_send_port.bind ( cpu->intrq_xport    );
        break;

      case Interconnect::DMA:
        cpu->init_socket.bind( nth->targ_socket );
        nth->init_socket.bind( rom->targ_socket );
        nth->init_socket.bind( ram->targ_socket );
        nth->init_socket.bind( ddr->targ_socket );
        nth->init_socket.bind( sth->targ_socket );
        sth->init_socket.bind( gio->targ_socket );
        sth->init_socket.bind( tmr->targ_socket );
        sth->init_socket.bind( pic->targ_socket );
        sth->init_socket.bind( dma->targ_socket );
        // Interrupts
        #if 0
        dma->intrq_port.bind    ( pic->irq_recv_xport );
        #endif
        tmr->intrq_port.bind    ( pic->irq_recv_xport );
        gio->intrq_port.bind    ( pic->irq_recv_xport );
        pic->irq_send_port.bind ( cpu->intrq_xport    );
        break;

      default:
        REPORT( FATAL, "Missing configuration." );
        break;
    }
  }

  ~Impl( void ) = default;

  Options* const options { nullptr };
};

void Top_module::end_of_elaboration( void )
{
  MESSAGE( "End of elaboration\n" );
  MEND( ALWAYS );
  Netlist dump;
}

void Top_module::start_of_simulation( void )
{
  Summary::starting_simulation();
  MESSAGE( "\n" );
  RULER('!');
  MESSAGE( "Start of simulation\n" );
  MESSAGE( "Clock period is " << pImpl->clk.period() );
  MEND( ALWAYS );
}

void Top_module::end_of_simulation( void )
{
  INFO( ALWAYS, "End of simulation\n" );
  Summary::finished_simulation();
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
