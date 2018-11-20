About Wallclock
===============

Some routines to measure real (the clock on the wall) and CPU execution time.
This code is platform independent, and has been tested on OS X, Linux, and
Windows.

Usage
-----

```cpp
  double wall0 = get_wall_time();
  double cpu0  = get_cpu_time();
```

Testing
-------

```sh
g++ -o wallclock.exe wallclock.cpp && ./wallclock.exe
```

### The end
