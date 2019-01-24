About `Memory_module`
=====================

Basic TLM 2.0 memory model.

Features
--------
- TLM 2.0 base protocol compliant
- Supports `Config_extn`
- Configurable depth
- Optionally read-only
- Supports DMI

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

The memory module implements a model suitable for things such as SRAM, DRAM,
ROM, or EPROM. It would need serious modifications to accommodate FLASH memory
modeling, which should probably be a separate implementation.

Block diagram
-------------
```
{:DELETE SECTION IF NOT USED:}
```

Usage Example
-------------

See `memory_test.cpp`

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH:}

### The end
<!-- vim:tw=78
-->
