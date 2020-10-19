#ifndef UART_REG_H
#define UART_REG_H

// Restricted to C 2011 syntax to allow use by embedded C developers

#include "common/macros.h"
#include <stdint.h>

// Bit masks
enum UartField : uint32_t
// Control
{ UART_CONFIGOK_LSB    =  0x1A
  , UART_CONFIGOK_BITS =  1
  , UART_CONFIGOK_MASK = MASK( UART_CONFIGOK )
, UART_FIFOERR_LSB     =  0x19
  , UART_FIFOERR_BITS   =  1
  , UART_FIFOERR_MASK   = MASK( UART_FIFOERR )
, UART_DATAERR_LSB     =  0x18
  , UART_DATAERR_BITS  =  1
  , UART_DATAERR_MASK  = MASK( UART_DATAERR )
, UART_RUNNING_LSB     =  0x17
  , UART_RUNNING_BITS  =  1
  , UART_RUNNING_MASK  = MASK( UART_RUNNING )
, UART_ENADMA_LSB      =  0x16
  , UART_ENADMA_BITS   =  1
  , UART_ENADMA_MASK   = MASK( UART_ENADMA )
, UART_ENAINTR_LSB     =  0x15
  , UART_ENAINTR_BITS  =  1
  , UART_ENAINTR_MASK  = MASK( UART_ENAINTR )
, UART_DATABITS_LSB    =  0x14
  , UART_DATABITS_BITS =  1
  , UART_DATABITS_MASK = MASK( UART_DATABITS )
, UART_STOPBITS_LSB    =  0x13
  , UART_STOPBITS_BITS =  1
  , UART_STOPBITS_MASK = MASK( UART_STOPBITS )
, UART_PARITY_LSB      =  0x12
  , UART_PARITY_BITS   =  1
  , UART_PARITY_MASK   = MASK( UART_PARITY )
, UART_ODD_LSB         =  0x11
  , UART_ODD_BITS      =  1
  , UART_ODD_MASK      = MASK( UART_ODD )
, UART_DEPTHEND_LSB    =  0x10
  , UART_DEPTHEND_BITS =  1
  , UART_DEPTHEND_MASK = MASK( UART_DEPTHEND )
, UART_FIFOUSED_LSB    =  0
  , UART_FIFOUSED_BITS =  8
  , UART_FIFOUSED_MASK = MASK( UART_FIFOUSED )
, UART_BAUDRATE_LSB    =  0
  , UART_BAUDRATE_BITS =  8
  , UART_BAUDRATE_MASK = MASK( UART_BAUDRATE )
// Misc. chicken bits
,   UART_PEEKADDR_LSB  =  0
  , UART_PEEKADDR_BITS =  7
  , UART_PEEKADDR_MASK = MASK( UART_PEEKADDR )
,   UART_RXSELECT_LSB  =  7
  , UART_RXSELECT_BITS =  1
  , UART_RXSELECT_MASK = MASK( UART_RXSELECT )
, UART_PEEKDATA_LSB    =  8
  , UART_PEEKDATA_BITS =  8
  , UART_PEEKDATA_MASK = MASK( UART_PEEKDATA )
, UART_TXBITS_LSB      =  16
  , UART_TXBITS_BITS   =  8
  , UART_TXBITS_MASK   = MASK( UART_TXBITS )
, UART_RXBITS_LSB      =  24
  , UART_RXBITS_BITS   =  8
  , UART_RXBITS_MASK   = MASK( UART_RXBITS )
};

// Convenience
typedef struct {      // Bits Posn Description
  bool      configok ;//   1  0x1A Configuration 0=>Error 1=>Ok -- Read-only
  bool      fifoerr  ;//   1  0x19 Transmitter overflow / Receiver underflow
  bool      dataerr  ;//   1  0x18 Bad parity
  bool      running  ;//   1  0x17 Enable function Tx/Rx
  bool      enadma   ;//   1  0x16 Enable DMA
  bool      enaintr  ;//   1  0x15 Enable interrupts
  bool      databits ;//   1  0x14 0 => 8, 1 => 7
  bool      stopbits ;//   1  0x13 0 => 1, 1 => 2
  bool      parity   ;//   1  0x12 Enable parity
  bool      odd      ;//   1  0x11 0 => even, 1 => odd
  bool      depthend ;//   1  0x10 Rx => full, Tx => empty - Read-only
  uint8_t   fifoused ;//   8  0x 8 number of occupied locations - Read-only
  uint8_t   baudrate ;//   8  0x 0 see Uart_baud
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

typedef struct {      // Addr Descripton
  uint32_t txctrl;    // 0x00 Control/status (see Uart_ctrl_t) 
  uint32_t txdmap;    // 0x04 DMA transmit pointer
  uint32_t txdmac;    // 0x08 DMA transmit count
  uint32_t txdata;    // 0x0C Transmit fifo (byte address) -- Write-only
  uint32_t rxctrl;    // 0x10 Control/status (see Uart_ctrl_t)
  uint32_t rxdmap;    // 0x14 DMA receive pointer
  uint32_t rxdmac;    // 0x18 DMA receive count
  uint32_t rxdata;    // 0x1C Receive fifo (byte address) -- Read-only
  // Chicken bits
  uint32_t txclocks;  // 0x20 # clocks per bit x # bits per char -- Read-only
  uint32_t rxclocks;  // 0x24 # clocks per bit x # bits per char -- Read-only
  uint32_t misc;      // 0x28 RXBITS:8 TXBITS:8 PEEKSEL:1 PEEDADR:7 PEEKDATA:8
  uint32_t fifostats; // 0x2C RXSIZE:8 RXAVG:8 TXSIZE:8 TXAVG:8  -- Read-only
} Uart_regs_t;

// Address offsets
enum UartAddr : uint64_t
{ UART_TXCTRL_REG    =  0X00
, UART_TXDMAP_REG    =  0X04
, UART_TXDMAC_REG    =  0X08
, UART_TXDATA_REG    =  0X0C
, UART_RXCTRL_REG    =  0X10
, UART_RXDMAP_REG    =  0X14
, UART_RXDMAC_REG    =  0X18
, UART_RXDATA_REG    =  0X1C
// Chicken bits - read-only      
, UART_TXCLOCKS_REG  = 0x20
, UART_RXCLOCKS_REG  = 0x24
, UART_MISC_REG      = 0x28
, UART_FIFOSTATS_REG = 0x2C
, UART_REGS_SIZE     = sizeof(Uart_regs_t)
};

#endif /*UART_REG_H*/
