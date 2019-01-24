About `Pic_module`
==================

Basic TLM 2.0 model of a Priority Interrupt Controller (PIC)

Features
--------
- TLM 2.0 base protocol compliant
- Supports `Config_extn`
- Effectively unlimited inputs
- Multiple target outputs (32)
- 8-bit Priority and masking

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better
- Transactions must carry `Cpuid_extn`

Functional Description
----------------------

The Priority Interrupt Controller is a simple TLM module for coalescing
interrupt sources into a single interrupt. This design is based on concepts
used in Arm's GIC.

Note: In the following, we document registers using bit fields with idea that
they are stored MSB first. Code does NOT depend on this, but when packing
registers, these are the assumptions.

Dependent on the number of incoming attached interrupts, there are the
following registers:

```cpp
struct Pic_Source_t
{
  uint32_t reserved : 21 ;  // TODO: ?edge/level sensitive?
  uint32_t pending  :  1 ;
  uint32_t active   :  1 ;  // read-only
  uint32_t enabled  :  1 ;
  uint32_t priority :  8 ;  // higher is bigger
};
```

There is also a concept of multiple targets (processors).  In order for this
to work, there may be up to the same number of bus (`targ_socket`) connections
as `intrq_port` connections. This design has up to 32 targets due to register
setup, but could easily be extended as needed.

Processors are not identified by the incoming bus port except to pass back AT
responses. Instead, processors are identified by the required `Cpuid_extn`.
This extension needs to be given a unique initiator id, hence it using the
object pointer of the module (i.e. `this`).

A priority queue per target is used to setup requests.

A status register contains:

```cpp
struct Pic_Target_t {
  uint32_t targetid : 16 ; //< Self-reference -- read-only
  uint32_t reserved :  6 ;
  uint32_t clearall :  1 ; //< clear queue for this interface
  uint32_t enabled  :  1 ; //< interface is active
  uint32_t mask     :  8 ; //< prevent lower-priorities
};
```

There are two registers for identification and configuration.
Configuration indicates the number of sources and targets.

```cpp
struct Pic_Config_t {
  uint32_t target_count : 16 ;
  uint32_t source_count : 16 ;
};
```

The following is the processor interface register set:

```cpp
struct Pic_regs_t
{
  uint32_t        next; //< read for next interrupt
  uint32_t        done; //< write when done servicing
  Pic_Target_t    target;
  // 21 reserved;
  //  1 enabled;    //< interface is active
  //  1 clearall;   //< clear queue for this interface
  //  8 mask;       //< prevent lower-priorities
  uint32_t        ident;  //< read-only
  uint32_t        config; //< read-only
  uint32_t        select; //< choose interrupt to interrogate or setup
  Pic_Source_t    source;
  // 21 reserved; 
  //  1 pending;
  //  1 active; //< Read-only
  //  1 enabled;
  //  8 priority;
  uint32_t        targets;
};
```

Access to the request queues is arbitrated so that if an
interrupt is serviced, duplicates are removed from other
target queues.

Block diagram
-------------

```
+------+   +------+        +------+
| src1 |   | src2 |  * * * | srcN |
+--v---+   +--v---+        +--v---+
   |          |               |
   '--------. | .--- * * * ---'
             \|/ Separate incoming interrupt source connections
              |
              |                    +--------+          
              |                    |        |           
        intrq_xport[N]         .---<  cpu1  <---------.
              |                |   |        |         |
              v                |   +--------+         |
      +-------v-------+        |                      |
      | intrq_chan[N] |        |   +--------+         |
      |               |   b    |   |        |         |
      |      pic   [M]<<- u ---x---<  cpu2  <-------x |
      |               |   s    |   +--------+       | |
      |  *Pic_module* |        :       *            | |
      +-------v-------+        :       *            | |
              v                :       *            | |
              |                |   +--------+       | |
         intrq_port[M]         |   |        |       | |
              |                '---<  cpuM  <---.   | |
              |                    |        |   |   | |
              |                    +--------+   |   | |
              '---------------------------------'...'-'
               Separate outgoing interrupt target connections

```

- N = # of interrupt generating sources
- M = # of interrupt receiving target CPU masters

Usage Example
-------------

```cpp
{:CODE SNIPPETS TO BE SUPPLIED:}
```

Files
-----

  Filename         | Purpose                                       
  ---------------- | ----------------------------------------------
  `ABOUT_PIC.md`   | Documentation in markdown                     
  `pic.cpp`        | Implements `Pic_module`                       
  `pic.hpp`        | Header needed to instantiate `Pic_module`     
  `pic_api.hpp`    | Provides programming conveniences for software
  `pic_reg.h`      | Provides register definitions for software    
  `cpu/cpuid_extn.hpp` | Declarations for Cpuid extension
  `cpu/cpuid_extn.hpp` | Implementation definitions for Cpuid extension

Notes
-----

- Reading the NEXT register has two possible returns:

  1. `PIC_INVALID_IRQ` indicates an error, which includes the spurious
     situation. When more than one target CPU is interrupted for a single
     interrupt, only the first one to read that particular interrupt from the
     NEXT register will get it. Other responders may get the `PIC_INVALID_IRQ`
     if there is nothing else outstanding of the appropriate priority level.
     In general this should not be treated as an error.

  2. A number representing a real interrupt configured in the memory map.

- Writing the DONE register will set the corresponding interrupt inactive even
  if the write originates from a CPU that has not made the interrupt active.

- Reading the NEXT and DONE registers works regardless of the interface's
  enable status Writing an unassigned interrupt to pending effectively creates
  a software generated interrupt.

- Setting the CLEARALL bit will cause the pending and active status of all
  interrupt sources to be cleared. Reading the target register will return the
  bit set for the next read and clear it for subsequent reads.

Implementation Details
----------------------

Each interrupt source may be represented with two flags, pending and active.
They are managed in a state machine that roughly looks as follows:

See Also
--------

* [`cpu/ABOUT_CPUID_EXTN.md`](file:ABOUT_CPUID_EXTN.md)

### The end
<!-- vim:tw=78
-->
