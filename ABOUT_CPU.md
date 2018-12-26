About the CPU
=============

This models a generic TLM 2.0 processor initiator unit.

Features
--------
- TLM 2.0 base protocol compliant
- Convenience functions and task support
- Interrupt input (1) 

Functional Description
----------------------

The CPU module model a generic intiator, but has features that make
it easy to:

1. Model simple transfers for basic testing
2. Create a simple ISS

Block diagram
-------------

```
         +-----+
         |     |
 IRQ -->[] CPU >-- TLM
         |     |
         +-----+
```

Implementation Details
----------------------

The CPU is a TLM initiator module is broken into several parts:

1. The bus connection via an internally provided `transport()`
   method implements communications to the "bus" via the
   `init_socket`. This is where protocol customizations carried
   out.

2. The `cpu_thread` uses the task manager to allow externally
   compiled behaviors to be developed using the `Cpu_if` API,
   which consists of generic bus read/write/get/put methods.

3. An interrupt input port.

See Also
--------
- Task Manager `ABOUT_TASK.md`

### The end
<!-- vim:tw=78
-->
