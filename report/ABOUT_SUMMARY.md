About `Summary class`
==========================

Convenience class for reporting a summary of SystemC statistics.

Features
--------
- {:FEATURE:}

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

Collects simulation statistics and provides a convenient way
to dump them from `sc_main`.

Usage Example
-------------

See `top/main.cpp`

Output Example
--------------

```
Info: /Doulos/Example/summary: 
################################################################################
Compilation information for build/platform:
  C++ std version: 201402 (ISO/IEC 14882:2014)
  SystemC version: 20181013
  TLM     version: 2.0.5_pub_rev-Accellera
  Compiled  using: Clang/LLVM version 10.0.0

Info: /Doulos/Example/summary: 
--------------------------------------------------------------------------------
Summary for build/platform:
  CPU elaboration time 0.002103 sec
  CPU simulation  time 0.01066 sec
   2 warnings
   0 errors
   0 fatals

################################################################################
Simulation PASSED
```


Files
-----
* report -- {:COMMENT1:}
* report -- {:COMMENT1:}

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH:}

See Also
--------

### The end
<!-- vim:tw=78
-->
