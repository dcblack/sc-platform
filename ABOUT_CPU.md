About the CPU
=============

This is a generic processor unit.

Block diagram
-------------

Implementation Details
----------------------

The CPU is a TLM initiator module is broken into several parts:

1. The bus connection via an internally provided `transport()`
   method implements communications to the "bus" via the
   `init_socket`. This is where protocol customizations carried
   out.

2. The cpu_thread uses the task manager to allow externally
   compiled behaviors to be developed using the Cpu_if API,
   which consists of generic bus read/write/get/put methods.

3. An interrupt input port.

### end
