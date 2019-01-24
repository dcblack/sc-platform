About `Cpuid_extn`
===================

TLM 2.0 transaction extension to identify the originating initiator in a transaction.
Used by `Pic_module`

Features
--------
- TLM 2.0 auto extension

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

The `Cpuid_extn` module provides a simple way to access information about
the originating SystemC module. In other words, it provides a reference to
the module where it was created. Although, it does not strictly require the
originator to be a TLM module, it is designed for use with TLM 2.0 as an
auto extension.

Some modules may require its presence in order to work properly. For example,
the `Pic_module` uses this to identify interrupt targets.

Note that once the Cpu

Usage Example
-------------

In the originating TLM module:
```cpp
tlm::tlm_generic_payload& trans { *memory_manager.allocate() };
trans.acquire();
trans.set_auto_extension( new Cpuid_extn( name() ) );
```

In the target TLM module:
```cpp
auto cpuid_ptr = trans.get_extension<Cpuid_extn>();
if( cpuid_ptr != nullptr ) REPORT( INFO, "Transaction received from " << cpuid_ptr->name() );
```

Testing
-----

{:TO BE SUPPLIED:}

Files
-----

  Filename                 | Purpose                  
  ------------------------ | -------------------------
  `ABOUT_CPUID_EXTN.md`   | Documentation in markdown
  `cpuid_extn.hpp`        | Declaration header
  `cpuid_extn.cpp`        | Definition implementation

Implementation Details
----------------------

Copy from silently does not copy if destination is set. This allows
proper use of this extension in bridges and adaptors.

See Also
--------
* [`ABOUT_PIC.md`](file:ABOUT_PIC.md)

### The end
<!-- vim:tw=78
-->
