#ifndef MEMORY_MAP_HPP
#define MEMORY_MAP_HPP

// Crude approach (port of NA means currently *UNDEFINED*)

#define NA MAX_ADDR

enum Memory_map : uint64_t
{ NTH_BASE = 0x0000'0000ull, NTH_DEPTH =   4*GB , NTH_PORT = NA //  -  
, STH_BASE = 0x4000'0000ull, STH_DEPTH =  64*KB , STH_PORT = NA //  -  
, CPU_BASE = 0xF000'0000ull, CPU_DEPTH =    128 , CPU_PORT = NA //  -  
, ROM_BASE = 0x0000'0000ull, ROM_DEPTH =  16*MB , ROM_PORT =  0 // NTH 
, RAM_BASE = 0x1000'0000ull, RAM_DEPTH = 512*KB , RAM_PORT =  1 // NTH 
, DDR_BASE = 0x2000'0000ull, DDR_DEPTH =   1*GB , DDR_PORT = NA // NTH 
, TMR_BASE = 0x4000'0000ull, TMR_DEPTH =     32 , TMR_PORT = NA // sth 
, GIO_BASE = 0x4000'1000ull, GIO_DEPTH =     32 , GIO_PORT = NA // sth 
, CON_BASE = 0x4000'2000ull, CON_DEPTH =     32 , CON_PORT = NA // sth 
, PIC_BASE = 0x4000'3000ull, PIC_DEPTH =   1*KB , PIC_PORT = NA // sth 
, DMA_BASE = 0x4000'4000ull, DMA_DEPTH =     32 , DMA_PORT = NA // sth 
, SPI_BASE = 0x4000'6000ull, SPI_DEPTH =     32 , SPI_PORT = NA // sth 
, SER_BASE = 0x4000'7000ull, SER_DEPTH =     32 , SER_PORT = NA // sth 
, DSK_BASE = 0x4008'0000ull, DSK_DEPTH =   1*KB , DSK_PORT = NA // NTH 
, NET_BASE = 0x4008'1000ull, NET_DEPTH =     32 , NET_PORT = NA // NTH 
, MMU_BASE = 0x4008'2000ull, MMU_DEPTH =     32 , MMU_PORT = NA // NTH 
, USB_BASE = 0x4008'3000ull, USB_DEPTH =     32 , USB_PORT = NA // NTH 
, VID_BASE = 0xC000'0000ull, VID_DEPTH =  16*MB , VID_PORT = NA // NTH 
, GPS_BASE = 0x0ull        , GPS_DEPTH =     32 , GPS_PORT = NA // spi 
, ENV_BASE = 0x100ull      , ENV_DEPTH =     32 , ENV_PORT = NA // spi 
, PTR_BASE = 0ull          , PTR_DEPTH =     32 , PTR_PORT = NA // usb 
};

#endif /*MEMORY_MAP_HPP*/
