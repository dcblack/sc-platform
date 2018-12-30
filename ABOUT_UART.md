About `Uart`
============

Basic serial port simulation.

Features
--------
- TLM base protocol compliant {:***DELETE LINE OR PLACEHOLDER ONLY AS APPROPRIATE***:}
- Supports `Config_extn` {:***DELETE LINE OR PLACEHOLDER ONLY AS APPROPRIATE***:}
- Communicates to a terminal via TCP sockets
- Simulates shift timing, but does NOT simulate actual bit shifting.

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

The usart module {:***TO BE SUPPLIED***:}

Block diagram
-------------

```
{:ILLUSTRATE CONTEXT OF THIS IN A GENERIC DESIGN -- ***TO BE SUPPLIED OR DELETED***:}
```

Registers
---------

```c
typedef struct {   // rA FUNC
  uint32_t txctrl; // x0 Control/status (see Uart_ctrl_t) 
  uint32_t txdmap; // x4 DMA transmit pointer
  uint32_t txdmac; // x8 DMA transmit count
  uint32_t txdata; // x4 Transmit fifo (byte address)
  uint32_t rxctrl; // x8 Control/status (see Uart_ctrl_t)
  uint32_t rxdmap; // x4 DMA receive pointer
  uint32_t rxdmac; // x8 DMA receive count
  uint32_t rxdata; // xC Receive fifo (byte address)
} Uart_regs_t;
```
Control/status fields
---------------------
```c
typedef struct {
  uint32_t running  : 1 ; // Enable function Tx/Rx
  uint32_t enadma   : 1 ; // Enable DMA
  uint32_t enaintr  : 1 ; // Enable interrupts
  uint32_t stopbits : 1 ; // 0 => 1, 1 => 2
  uint32_t parity   : 1 ; // Enable parity
  uint32_t odd      : 1 ; // 0 => even, 1 => odd
  uint32_t capacity : 1 ; // Rx => full, Tx => empty
  uint32_t depth    : 4 ; // number of occupied locations
  uint16_t rate;    : 8 ; // see table
} Uart_ctrl_t;

Syntax
------

See header comments for information on methods.

Usage Example
-------------

```cpp
{:CODE SNIPPETS ***TO BE SUPPLIED***:}
```

Testing
-----

{:***TO BE SUPPLIED*** -- POSSIBLY AS FOLLOWS -- DELETE THIS LINE WHEN CORRECT:}
```sh
g++ -std=c++14 -DUART_EXAMPLE -o usart.exe usart.cpp && ./usart.exe
```

Files
-----

  Filename         | Purpose                  
  ---------------- | -------------------------
  `ABOUT_UART.md`  | Documentation in markdown
  `uart.hpp`       | {:COMMENT1:}             
  `uart.cpp`       | {:COMMENT1:}             

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH:}

See Also
--------

### The end
<!-- vim:tw=78
-->
