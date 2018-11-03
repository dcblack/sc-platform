About `Timer_module`
====================

Functional Description
----------------------

The timer module may implement 1 to 16 timers. The number implemented may be read
from the QTY field of the status register of any timer.

Each timer contains four registers:

1. Control/status (32 bits)
2. Trigger value (64 bits)
3. Current value (64 bits)
4. Pulse width (32 bits)

Status consists of a number of fields and single bit flags:

* QTY indicates the number of implemented timers
* SCALE indicates the clock scaling factor for a particular timer
* IRQ indicates the interrupt status of all implemented timers
* RELOAD bit indicates whether timer should reload the trigger

Interrupt status for a respective timer is automatically cleared after reading
its status register.

There are three word (32-bit ) address offsets used to access
control/status:

0: Read for status - read-only
4: Control clear - clears any modifiable bit that is set to 1. Effectively `STATUS &= ~(VALUE);`
8: Control set - sets any modifiable bit that is set to 1. Effectively `STATUS |= VALUE;`

Timers have three possible states:

*  Stopped
*  Running
*  Paused

Running timers increment on every clock tick/scale when running.

There is a single interrupt output. Determining which timer(s) caused the
timer interrupt is determined by reading the status register.

The timer IRQ mask field of any timer will indicate the interrupt status of
all timers.

Status Register
---------------

 Control/status register
 Bit | Meaning                      | Notes
 --: | ---------------------------- | -----
  31 | IRQ15 interrupt active       | Read-only 
  30 | IRQ14 interrupt active       | Read-only
  29 | IRQ13 interrupt active       | Read-only
  28 | IRQ12 interrupt active       | Read-only
  27 | IRQ11 interrupt active       | Read-only
  26 | IRQ10 interrupt active       | Read-only
  25 | IRQ9  interrupt active       | Read-only
  24 | IRQ8  interrupt active       | Read-only
  23 | IRQ7  interrupt active       | Read-only
  22 | IRQ6  interrupt active       | Read-only
  21 | IRQ5  interrupt active       | Read-only
  20 | IRQ4  interrupt active       | Read-only
  19 | IRQ3  interrupt active       | Read-only
  18 | IRQ2  interrupt active       | Read-only
  17 | IRQ1  interrupt active       | Read-only
  16 | IRQ0  interrupt active       | Read-only
  15 | Reload on match              | 
  14 | One-shot/Continuous          | 
  13 | Run/Stop                     | 
  12 | Enable/Disable Interrupt     | 0 = disabled, 1 = enabled
  11 | Paused                       | 
  10 | Reserved                     | Read as zero/Write ignored
   9 | Reserved                     | Read as zero/Write ignored
   8 | Reserved                     | Read as zero/Write ignored
   7 | Scale[3] - Scale clock       | 
   6 | Scale[2]                     | 
   5 | Scale[1]                     | 
   4 | Scale[0]                     | 
   3 | Timers[3] - Number of timers | Read-only
   2 | Timers[2]                    | Read-only
   1 | Timers[1]                    | Read-only
   0 | Timers[0]                    | Read-only

Files
-----

  Filename         | Purpose
  --------         | -------
  `ABOUT_TIMER.md` | Documentation in markdown
  `timer.cpp`      | Implements `Timer_module`
  `timer.hpp`      | Header needed to instantiate `Timer_module`
  `timer_api.hpp`  | Provides programming conveniences for software
  `timer_reg.hpp`  | Provides register definitions for software
  `timer_beh.cpp`  | Implements simple `Timer` behavior
  `timer_beh.hpp`  | Header needed to instantiate `Timer`

Implementation Design Details
-----------------------------


### The end
