About Task Manager
==================

This abstraction allows threads to call "tasks" that implement behaviors in
separate compilation units. Typically, this could be used to implement tests for
a processor. Thus there might be three files: `memory_test.cpp`,
`timer_test.cpp`, and pic_test.cpp.  It keeps the clutter of TLM out of the test
code.

Usage
-----

The initiator needs to register itself.

```cpp
#include <systemc>
#include "report.hpp"
#include "cpu.hpp"
#include "tasks.hpp"
struct Initiator : sc_module {
  string mgr_name { "cpu" };
  Task_manager my_task_mgr{ mgr_name, this}; // allow multiple thread access
  void main_thread {
    Task_map_t task { cpu_task_mgr.tasks() };
    // Execute a single task
    auto task_name = "mem_test";

    if ( task.count( task_name ) == 1 )
    {
      INFO( MEDIUM, "Executing " << task_name );
      task[task_name]();
    }
    else
    {
      REPORT( ERROR, "No such task '" << task_name << "'" );
    }
  }
};
```

```cpp
#include "cpu.hpp"
#include "task.hpp"
void mem_test(void);
namespace {
  Task_manager mgr { "cpu", "mem_test", &mem_test };
  void mem_test(void)
  {
    Cpu_module* const cpu{ dynamic_cast<Cpu_module*>(mgr.obj) };
    sc_assert( cpu != nullptr );
    cpu->write32( RAM_BASE, 42 );
    ...
  }
}
```
