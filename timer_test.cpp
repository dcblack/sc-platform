#include "cpu.hpp"
#include "report.hpp"
#include "task.hpp"
#include "memory_map.hpp"
#include "timer_reg.hpp"
#include "timer_api.hpp"

namespace {
  const char* const MSGID{ "/Doulos/Example/timer_test" };
}
using namespace std;

#define VALUE(v) DEC << int(v) << HEX << " (" << int(v) << ")"
#define TEST_TIMER(t) do {                                        \
  MESSAGE( "Testing timer " << t.timer() << "\n" ); \
  MESSAGE( "  " << (t.is_running()?"Running.":"Halted.")<<"\n" );   \
  MESSAGE( "  Current status is " << HEX << t.status() << "\n" ); \
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

  MESSAGE( "\n" );
  RULER( 'T' );
  INFO( MEDIUM, "Testing timers" );

  // Basic write/read
  Addr_t timer_addr = TMR_BASE;
  uint32_t timer_send { 0x0'F00Dul };
  uint32_t timer_recv { 0xDEAD'BEEFul };
  cpu->write32( timer_addr + TIMER_TRIG_LO_REG, timer_send );
  INFO( MEDIUM, "Write timer_trig with " << VALUE(timer_send) );
  cpu->read32( timer_addr + TIMER_TRIG_LO_REG, timer_recv );
  INFO( MEDIUM, "Read timer_trig_lo and got " << VALUE(timer_recv) );

  Timer_api t0{ *cpu };
  t0.setup( 10 );
  TEST_TIMER( t0 );
  t0.start();
  cpu->clk.wait(50);
  TEST_TIMER( t0 );

  MESSAGE( "\n" );
  RULER( '-' );
  MEND(MEDIUM);
  Timer_api t1{ *cpu };
  t1.setup( 30 );
  TEST_TIMER( t1 );
  t1.start();

  cpu->clk.wait( 150 );
  TEST_TIMER( t0 );
  TEST_TIMER( t1 );
}
