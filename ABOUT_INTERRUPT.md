About `Interrupt_chan`
==========================

Simple interrupt channel and corresponding interface.

Features
--------
- Not dependent on TLM
- Edge triggered only
- Named sources (default originating module's instance name)

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

The interrupt channel supports the idea of delayed notifications and
many notifications may be outstanding.

Block diagram
-------------

```
+-------------------+   +-------------------+
|                   |   |                   |
| interrupt source  |   | interrupt source  |
|                   |   |                   |
+--------[v]--------+   +--------[v]--------+
          |                       |
          '-x---------------------'
            |
+----------(v)--------------------+
|           |                     |
|           v                     |
| +---------O---------+           |
| | Interrupt_recv_if | Interrupt |
| |                   | _debug_if |
| | Interrupt_channel |O>--       |
| |                   |           |
| | Interrupt_send_if |           |
| +---------O---------+           |
|           v                     |
|           |                     |
|                                 |
|   interrupt target              |
|                                 |
+---------------------------------+
                      
```

Syntax
------

See comments in `interrupt_if.hpp`

Usage Example
-------------

```cpp

SC_MODULE( Source ) {
  void thread()
  {
    ...
    irq_port->notify( name(), delay );
    ...
  }
};

SC_MODULE( Target ) {
  sc_export< Interrupt_send_if > irq_xport;
  Interrupt_channel irq_chan;
  SC_CTOR( Target )
  {
    irq_port.bind( irq_chan.send_if );
    SC_THREAD( irq_thread );
  }
  void irq_thread() {
    for(;;) {
      irq_chan.recv_if.wait();
      // Careful to call get once per interrupt
      string source = irq_chan.recv_if.get_next();
      REPORT( INFO, name() << " received interrupt"
              << " from " << source
              << " at "   << sc_time_stamp()
      );
      ...
    }
};

```

Testing
-------

Eventually...

```sh
g++ -std=c++14 -DINTERRUPT_EXAMPLE -o interrupt.exe interrupt.cpp && ./interrupt.exe
```

Files
-----

  Filename                   | Purpose                  
  -------------------------- | -------------------------
  `ABOUT_INTERRUPT.md`       | Documentation in markdown
  `interrupt_if.hpp`         | Interfaces               
  `interrupt.hpp`            | Channel implementation
  `interrupt.cpp`            | Channel unit testing   

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH:}

See Also
--------

`interrupt_test.cpp`

### The end
<!-- vim:tw=78
-->
