About `Gpio`
===================

General Purpose Input and Output.

Features
--------
- TLM base protocol compliant
- Supports `Config_extn`
- Input data may be fed via a time annotated file.
- Output data may be captured in a time annotated output file or simply dumped into standard out.
- Interrupt port does not need to be connected unless it will be used.
- GPIO ports do not need to be connected unless they are used.

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

The gpio module provides a number of `sc_signal` channels that are
configurable as inputs or outputs. By default they come up configured as
inputs. This module also provides for the ability to stream the data in either
or both directions from up to two file streams. One for input and the other
for output.

The base design supports 64 pins, but in principle could easily be expanded to
support multiples. Sixty-four was chosen due to the simplicity of software writing
64-bit data types (i.e. `uint64_t`).


In the following description, it is understood that all registers are 64-bits,
with each bit affecting the corresponding GPIO "pin". For example, if you want to
set pin 2 to be an output, then you need to set bit 2 of `pindirn` to zero.

Changed pins may be determined by first configuring the type of change (`pinrise`
or `pinfall`, and then reading the `datachg` register, which is sticky. To clear
`datachg` status, write 1's to corresponding bits.
Inputs may also be configured to generate an interrupt if a change occurs regardless
of whether the `datachg` was already set.

I/O is established using the `Gpio_extn` attached to any payload.

Block diagram
-------------

```
            bus
             |    @ intrq
       +-----v-----+
       |           |
       |   Gpio    |
       |           |
       +vvvv^^^^@@@+
        |||||||||||

sc_export<sc_signal_inout_if>

```

Registers
---------

```c
typedef struct {    // rA FUNC
  uint64_t pindirn; // x0 0=out, 1=in (tri)
  uint64_t pinintr; // x1 1=interrupt
  uint64_t pinpull; // x2 1=pull
  uint64_t pulldir; // x3 0=down/1=up
  uint64_t datainp; // x4 =
  uint64_t dataout; // x5 =
  uint64_t dataset; // x6 |=
  uint64_t dataclr; // x7 &= ~
  uint64_t datainv; // x8 ^=
  uint64_t datachg; // x9 value changed / clear with write
  uint64_t dataena; // xA write 1's to clear changed bits
  uint64_t pinrise; // xB 1's indicate rise detection
  uint64_t pinfall; // xC 1's indicate fall detection
  uint64_t reservD; // xD
  uint64_t reservE; // xE
  uint64_t reservF; // xF
} Gpio_regs_t;
```

Usage Example
-------------

```cpp
{:CODE SNIPPETS ***TO BE SUPPLIED***:}
```

Testing
-----

{:***TO BE SUPPLIED*** -- POSSIBLY AS FOLLOWS -- DELETE THIS LINE WHEN CORRECT:}
```sh
g++ -std=c++14 -DGPIO_EXAMPLE -o gpio.exe gpio/gpio.cpp && ./gpio.exe
```

Files
-----

  Filename                 | Purpose                  
  ------------------------ | -------------------------
  `gpio/ABOUT_GPIO.md`   | Documentation in markdown
  `gpio/gpio.hpp`        | Declaration header
  `gpio/gpio.cpp`        | Definition implementation
  `gpio/gpio_reg.h`      | Register constants

See also
--------

- `ABOUT_GPIO_EXTN.md`

Implementation Details
----------------------


### The end
<!-- vim:tw=78
-->
