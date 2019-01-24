About `sc_main`
===============

Features
--------
- Guarantees `end_of_simulation` will be run
- Supports expanding reporting
- Provides execution summary
- Returns PASS/FAIL status

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

This is an advanced implementation of `sc_main` that provides
better reporting than typical implementations. Assumes existance
of and instantiates `Top_module`.

Files
-----
* main.cpp

Implementation Details
----------------------

Fairly straight-forward using exception trapping and report/summary
support routines.

See Also
--------
* `ABOUT_REPORT.md`
* `ABOUT_SUMMARY.md`
* `ABOUT_WALLCLOCK.md`

### The end
<!-- vim:tw=78
-->
