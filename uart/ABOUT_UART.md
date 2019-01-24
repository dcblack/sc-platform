About `Uart`
============

Basic serial port simulation.

Features
--------
- TLM base protocol compliant {:***DELETE LINE OR PLACEHOLDER ONLY AS APPROPRIATE***:}
- Supports `Config_extn` {:***DELETE LINE OR PLACEHOLDER ONLY AS APPROPRIATE***:}
- Three forms of connections (must choose ONE):
  + Virtual - Communicates to a terminal via TCP sockets
  + Fast - Uses fast parallel transfer
  + Slow - Detailed serial conversion
- Independently configurable transmit (TX) and receive (RX)
  + 16 deep FIFOs
  + 7 or 8 data bits, 1 or 2 stop bits, parity
  + 15 different data rates (including "infinite" for fast Virtual testing)
  + Status: FIFO full/empty, current depth
  + Interrupt enable
  + Errors: FIFO over/under, parity, configuration

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
            bus
             |    @ intrq
       +-----v-----+
       |           |
       |   Uart    > Virtual
       |           < Port
       |  TR   TR  |
       +--v^---v^--+
          ||   ||
      Serial   Parallel
        Port   Port

Note: Only one Port may be used for a given simulation configuration.
```

Registers
---------

```c
typedef struct {      // Addr Descripton
  uint32_t txctrl;    // 0x00 Control/status (see Uart_ctrl_t) 
  uint32_t txdmap;    // 0x04 DMA transmit pointer
  uint32_t txdmac;    // 0x08 DMA transmit count
  uint32_t txdata;    // 0x0C Transmit fifo (byte address) -- Write-only
  uint32_t rxctrl;    // 0x10 Control/status (see Uart_ctrl_t)
  uint32_t rxdmap;    // 0x14 DMA receive pointer
  uint32_t rxdmac;    // 0x18 DMA receive count
  uint32_t rxdata;    // 0x1C Receive fifo (byte address) -- Read-only
  // Chicken bits - for testing
  uint32_t txclocks;  // 0x20 # clocks per bit x # bits per char -- Read-only
  uint32_t rxclocks;  // 0x24 # clocks per bit x # bits per char -- Read-only
  uint32_t misc;      // 0x28 RXBITS:8 TXBITS:8 PEEKSEL:1 PEEDADR:7 PEEKDATA:8
  uint32_t fifostats; // 0x2C RXSIZE:8 RXAVG:8 TXSIZE:8 TXAVG:8  -- Read-only
} Uart_regs_t;
```
Control/status fields
---------------------
```c
typedef struct { // Bits Posn Description
  bool configok ;//   1  0x1A Configuration 0=>Error 1=>Ok -- Read-only
  bool fifoerr  ;//   1  0x19 Transmitter overflow / Receiver underflow
  bool dataerr  ;//   1  0x18 Bad parity
  bool running  ;//   1  0x17 Enable function Tx/Rx
  bool enadma   ;//   1  0x16 Enable DMA
  bool enaintr  ;//   1  0x15 Enable interrupts
  bool databits ;//   1  0x14 0 => 8, 1 => 7
  bool stopbits ;//   1  0x13 0 => 1, 1 => 2
  bool parity   ;//   1  0x12 Enable parity
  bool odd      ;//   1  0x11 0 => even, 1 => odd
  bool depthend ;//   1  0x10 Rx => full, Tx => empty - Read-only
  int  fifoused ;//   8  0x 8 number of occupied locations - Read-only
  int  baudrate ;//   8  0x 0 see Uart_baud
} Uart_ctrl_t;

enum Uart_baud
{ UART_BAUDINF=0 // byte per CPU clock
, UART_BAUD110
, UART_BAUD300
, UART_BAUD600
, UART_BAUD1200
, UART_BAUD2400
, UART_BAUD4800
, UART_BAUD9600
, UART_BAUD14400
, UART_BAUD19200
, UART_BAUD38400
, UART_BAUD57600
, UART_BAUD115200
, UART_BAUD128000
, UART_BAUD256000
, UART_BAUD_SIZE
};

```

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
