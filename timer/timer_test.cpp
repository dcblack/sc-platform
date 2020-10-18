#include "cpu/cpu.hpp"
#include "report/report.hpp"
#include "cpu/task.hpp"
#include "bus/memory_map.hpp"
#include "timer/timer_reg.h"
#include "timer/timer_api.hpp"

namespace {
  const char* const MSGID{ "/Doulos/Example/timer_test" };
}
using namespace std;

#define VALUE(v) STREAM_DEC << int(v) << STREAM_HEX << " (" << int(v) << ")"
#define TEST_TIMER(mesg,t) do {                                        \
  MESSAGE( mesg << " timer " << t.timer() << "\n" ); \
  MESSAGE( "  " << (t.is_running()?"Running.":"Halted.")<<"\n" );   \
  MESSAGE( "  Current status is " << STREAM_HEX << t.status() << "\n" ); \
  MESSAGE( "  Current count  is " << VALUE(t.value()) << "\n" );  \
  MEND( MEDIUM );                                                 \
} while(0)

void timer_test(void);
namespace {
  // Connects to the task manager
  Task_manager mgr { "cpu", "timer_test", &timer_test };
}
void timer_test(void)
{
  Cpu_module* cpu{ dynamic_cast<Cpu_module*>(mgr.obj()) };
  sc_assert( cpu != nullptr );

  const Addr_t TMR_BASE{ cpu->find_address( "top.tmr" ) };

  MESSAGE( "\n" );
  RULER( 'B' );
  MESSAGE( "Timer constants\n" );
  MESSAGE( "  TIMER_QTY_MASK   =" << STREAM_HEX << TIMER_QTY_MASK );
  MESSAGE( "  TIMER_SCALE_MASK =" << STREAM_HEX << TIMER_SCALE_MASK );
  MESSAGE( "  TIMER_IRQ_MASK   =" << STREAM_HEX << TIMER_IRQ_MASK );
  MEND( MEDIUM );

  MESSAGE( "\n" );
  RULER( 'T' );
  INFO( MEDIUM, "Testing timers" );

  // Basic write/read
  Addr_t timer_addr = TMR_BASE;
  uint32_t timer_send { 0x0'F00Dul };
  uint32_t timer_recv { 0xDEAD'BEEFul };
  cpu->write32( timer_addr + TIMER_LOAD_LO_REG, timer_send );
  INFO( MEDIUM, "Write timer_load with " << VALUE(timer_send) );
  cpu->read32( timer_addr + TIMER_LOAD_LO_REG, timer_recv );
  INFO( MEDIUM, "Read timer_load_lo and got " << VALUE(timer_recv) );

  Timer_api t0{ *cpu, TMR_BASE };
  t0.setup( 10 );
  TEST_TIMER( "Setup for 10 on", t0 );
  t0.start();
  TEST_TIMER( "Started", t0 );
  cpu->clk.wait(50);
  TEST_TIMER( "Waited 50 on", t0 );

  MESSAGE( "\n" );
  RULER( '-' );
  MEND(MEDIUM);
  Timer_api t1{ *cpu, TMR_BASE };
  t1.setup( 30 );
  TEST_TIMER( "Setup for 30 on", t1 );
  t1.start();

  cpu->clk.wait( 70 );
  t0.stop();
  TEST_TIMER( "Final status for", t0 );
  TEST_TIMER( "Final status for", t1 );
  t1.stop();
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
