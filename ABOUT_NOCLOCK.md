About `no_clock`
================

The `no_clock` object simplifies high-level modeling of clocks without
incurring the context switching overhead of a real clock.  It does so by
providing a number of methods that calculate timing offsets and reducing
multiple clock delays to a single wait.  This contrasts with traditional
clocks that issues millions of unused cycles that slow down simulation due
to the overhead of context switching.

To simplify the usage model, `no_clock` is designed to have a similar syntax
to a real `sc_clock`. This should make it easier for designers to adopt. In
fact, you should be able to replace `no_clock` with `sc_clock` at any time as
the design is refined without much change to the underlying code.

New concepts include sampling and setting time offsets. This simplifies
modeling setup & hold rules and aids verification.

Below are declarations and corresponding timing diagrams that may be useful
to understanding the design.  We also provide a number of utility methods
in the `no_clock` class as an aid to modeling.

```cpp
 const sc_core::sc_time(1.0,SC_NS) ns;
 no_clock CLK1("CLK1",/*period*/10_ns,/*duty*/0.5,/*offset*/0_ns,/*1stpos*/true ,/*smpl*/1*ns,/*chg*/5*ns);
 no_clock CLK2("CLK2",/*period*/12_ns,/*duty*/0.3,/*offset*/1_ns,/*1stpos*/false,/*smpl*/7*ns,/*chg*/9*ns);
```

Examples of above definitions impact to the virtual clocks they define.

```
 |                                |                                    |
 |       _0123456789_123456789_1  |       _123456789_123456789_123456  |
 |        :____     :____     :_  |       _:        ___:        ___:   |
 |  CLK1 _|    |____|    |____|   |  CLK2  |_______|:  |_______|:  |_  |
 |        :    :    :    :    :   |        :  :  :     :  :  :     :   |
 |  DATA  :s   c    :s   c    :   |        :  s  c     :  s  c     :   |
 |        ::   :    ::   :    :   |        :  :  :     :  :  :     :   |
 |  Time  0:   :   10:   :   20   |  Time  1      8 10 13    20 22 25  |
 |         1   5    11  15        |        :<--7->: :                  |
 |                                |        :<---9-->:                  |
```

In diagram, (s)ample and (c)hange are simply abbreviations.

Finally, we implement the concept of global clocks as a convenience. This
allows modeling without the burden of connecting clocks, which may be
deferred to implementation. Each instantiation should maintain local
references to the global clocks it uses.

Note that `no_clock` does not inherit from `sc_object,` and may thus be
instantiated at anytime. This is unlike `sc_core::sc_clock`, which is a
proper piece of SystemC hardware.

Usage Example
-------------

```cpp
no_clock clk { "clk", 100_MHz };

void main_thread( void )
{
  // Delay 500 clocks
  clk.wait( 500 );

  // Calculate
  sc_time delay;
  delay += 
}
```

### The end
