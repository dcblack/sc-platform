About the PIC
=============

The Priority Interrupt Controller is a simple TLM module for
coalescing interrupt sources into a single interrupt. This
design is based on concepts used in Arm's GIC.

Note: In the following, we document registers using bit fields
with idea that they are stored MSB first. Code does NOT depend
on this, but when packing registers, these are the assumptions.

Dependent on the number of incoming attached interrupts, there
are the following registers:

```c++
struct IrqReg
{
  uint32_t:8 reserved1;
  uint32_t:8 targets;
  uint32_t:5 reserved2;
  uint32_t:1 pending;
  uint32_t:1 active;
  uint32_t:1 enabled;
  uint32_t:8 priority; // higher is bigger
};
```

There is also a concept of multiple targets (processors) up to 8.
In order for this to work, there must be the same number
of bus (`targ_socket`) connections as `intrq_port` connections.

A priority queue per target is used to setup requests.

The following is the processor interface register set:

```cpp
struct CpuReg
{
   uint32_t request; //< read for next interrupt
   uint32_t acknowledge; //< write when done servicing
   struct status {
     uint32_t:8 reserved1;
     uint32_t:6 reserved2;
     uint32_t:1 enabled; //< interface is active
     uint32_t:1 clearall; //< clear queue for this interface
     uint32_t:8 mask;
   };
   uint32_t select; //< choose interrupt to interrogate or confi
   struct config {
     uint32_t:8 reserved1;
     uint32_t:8 targets;
     uint32_t:5 reserved2;
     uint32_t:1 pending;
     uint32_t:1 active;
     uint32_t:1 enabled;
     uint32_t:8 priority;
   };
};
```

Access to the request queues is arbitrated so that if an
interrupt is serviced, duplicates are removed from other
target queues.

Block diagram
-------------
```
      intrq_xport[n]
            |
            v
    +-------v-------+
    | intrq_chan[n] |
b   |               |
u ->>[m]            |
s   |               |
    |               |
    +-------v-------+
            v
            |
       intrq_port[m]
```

Usage Example
-------------

```cpp
{:CODE SNIPPETS:}
```

Implementation
--------------

Internally, `sc_vector<Interrupt> intrq_chan` has one
entry per incoming `intrq_xport` binding. Received interrupts
place each managed interrupt into one of four enumerated states:

```cpp
enum class IrqState
{ IDLE
, PENDING
, ACTIVE
, ACTIVE_N_PENDING
};
```

### end
