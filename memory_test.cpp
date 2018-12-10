#include "cpu.hpp"
#include "report.hpp"
#include "task.hpp"
#include "hexfile.hpp"
#include "memory_map.hpp"

namespace {
  const char* const MSGID{ "/Doulos/Example/memory_test" };
}
using namespace std;

#define VALUE(v) DEC << int(v) << HEX << " (" << int(v) << ")"
#define TEST_RAM(w,addr,value) do {                             \
  cpu->write##w( RAM_BASE+addr, value );                        \
  data##w = int##w##_t(~value);                                 \
  cpu->read##w ( RAM_BASE+addr, data##w );                      \
  if( value != data##w ) REPORT( WARNING, "Data mismatch!" );   \
  MESSAGE( "wrote " << addr << ":" << VALUE(value) );           \
  MESSAGE( "read "  << addr << ":" << HEX << int(data##w) );    \
  MEND( MEDIUM );                                               \
} while(0)

void memory_test(void);
namespace {
  // Connects to the task manager
  Task_manager mgr { "cpu", "memory_test", &memory_test };
}

void memory_test(void)
{
  Cpu_module* cpu{ dynamic_cast<Cpu_module*>(mgr.obj()) };
  sc_assert( cpu != nullptr );

  const Addr_t RAM_BASE{ cpu->find_address( "top.ram" ) };

  MESSAGE( "\n" );
  RULER( 'M' );
  INFO( MEDIUM, "Testing writing/reading memory" );

  uint32_t data32;
  uint16_t data16;
  uint8_t  data8;
  TEST_RAM( 8,  0, 0xEF );
  TEST_RAM( 8,  1, 0xBE );
  TEST_RAM( 16, 2, 0xCAFE );
  data32 = 0u;
  cpu->read32( RAM_BASE + 0, data32 );
  INFO( MEDIUM, "read 0:" << VALUE(data32) );
  vector<short> v1( 8 );
  v1 = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 };
  INFO( MEDIUM, "v1 = " << HEX << v1 );
  cpu->write( RAM_BASE + 0x100, v1 );
  vector<int> v2( 4 );
  cpu->read( RAM_BASE + 0x100, v2 );
  INFO( MEDIUM, "v2 = " << v2 );

  INFO( MEDIUM, "Testing hexfile functions" );

  for ( int i = 3000; i < 3050; ++i ) {
    v2.push_back( i );
  }

  hexfile::dump( 0, v2 );
  hexfile::save( "test.dat", 100, v2 );
  vector<uint8_t> v3;
  Addr_t a = hexfile::load( "test.dat", v3 );
  INFO( MEDIUM, "a=" << a << " v3.size=" << v3.size() );
  hexfile::dump( a, v3 );
}
