About wallclock support
=======================

Features
--------
- TLM 2.0 base protocol compliant
- Supports `Config_extn`
- Works on Linux, OS X and Windows

Requirements
------------
- SystemC 2.3.2 or better
- C++98 or better

Functional Description
----------------------

The wallclock module provides two platform independent global functions
dealing with processor execution time and wall clock time. From the header
file we have:

```c
// get_wall_time() returns wall wall clock time since epoch in seconds - reasonable for simulator
// performance measurements. Works for Linux, OS X and Windows.
double get_wall_time(void);

// get_cpu_time() returns wall CPU clock time since start of program in seconds - reasonable for simulator
// performance measurements. Does not include sub-processes (i.e. system() calls).
// Works for Linux, OS X and Windows.
double get_cpu_time(void);
```

Usage Example
-------------

```cpp
#include "top/wallclock.hpp"
#include <iostream>
int main( void )
{
  double wall_begin = get_wall_time();
  double cpu_begin = get_cpu_time();
  do_something_hard();
  double wall_end = get_wall_time();
  double cpu_end = get_cpu_time();

  std::cout << "Elaped wall time was " << (wall_end - wall_begin)  << " seconds." << std::endl
  std::cout << "Processor CPU execution time was " << (cpu_end - cpu_begin)  << " seconds." << std::endl

  return 0;
}
```

Testing
-------

```sh
g++ -DTEST_WALLCLOCK -o wallclock.exe wallclock.cpp && ./wallclock.exe
```

Files
-----
* wallclock.hpp
* wallclock.cpp

### The end
<!-- vim:tw=78
-->
