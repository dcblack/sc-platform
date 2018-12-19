////////////////////////////////////////////////////////////////////////////////
//
//  #     # ##### #     #  ####  #####  #     #        #     #    #    #####      
//  ##   ## #     ##   ## #    # #    #  #   #         ##   ##   # #   #    #     
//  # # # # #     # # # # #    # #    #   # #          # # # #  #   #  #    #     
//  #  #  # ##### #  #  # #    # #####     #           #  #  # #     # #####      
//  #     # #     #     # #    # #  #      #           #     # ####### #          
//  #     # #     #     # #    # #   #     #           #     # #     # #          
//  #     # ##### #     #  ####  #    #    #    ###### #     # #     # #          
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MEMORY_MAP_HPP
#define MEMORY_MAP_HPP

#include "common.hpp"
#include <string>
#include <ostream>
#include <map>
#include <memory>
#include <list>

#ifdef TRIVIAL_MAP
#define NA MAX_ADDR
enum Memory_map : uint64_t
{ NTH_BASE = 0x0000'0000ull, NTH_DEPTH =   4*GB , NTH_PORT = NA //  -  
, STH_BASE = 0x4000'0000ull, STH_DEPTH =  64*KB , STH_PORT = NA //  -  
, CPU_BASE = 0xF000'0000ull, CPU_DEPTH =    128 , CPU_PORT = NA //  -  
, ROM_BASE = 0x0000'0000ull, ROM_DEPTH =  96*MB , ROM_PORT =  0 // NTH 
, RAM_BASE = 0x1000'0000ull, RAM_DEPTH = 128*KB , RAM_PORT =  1 // NTH 
, DDR_BASE = 0x2000'0000ull, DDR_DEPTH = 256*MB , DDR_PORT = NA // NTH 
, TMR_BASE = 0x4000'0000ull,                      TMR_PORT =  2 // sth 
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
#else

#define UNASSIGNED 0
#define UNLIMITED  0
struct Target_info {
  // Initial values chosen to force initialization discovery if need be
  Port_t      port  { BAD_PORT   }; // get from probe
  Addr_t      base  { BAD_ADDR   }; // get from YAML
  Addr_t      last  { 0          }; // computed
  Depth_t     size  { UNASSIGNED }; // YAML unless 0
  int         irq   { UNASSIGNED }; // get from YAML
  std::string name  { ""         }; // get from YAML
  std::string kind  { ""         }; // get from probe
};
std::ostream& operator<<( std::ostream& os, const Target_info& rhs );
using Address_map = std::map< Addr_t, Target_info, std::greater<Addr_t> >;

// Singleton class
struct Memory_map
{
  // Local types
  using Target_ptr = std::unique_ptr<Target_info>;
  enum { BAD_TARG = -1 };
  using Targ_t = int;
  using Target_map = std::map< std::string, Target_info >;
  enum class Category { unknown, initiator, interconnect, router, target };
  struct Origin_info {
    std::string name;
    Category    category;
    Addr_t      base;
    Depth_t     size{ UNLIMITED };
    Target_map  target_map; //< port => info
  };
  using Origin_map = std::map< std::string, Origin_info >; // origin => map

  // Public Methods
  static Address_map get_address_map( const std::string & origin_name );
  static Target_info get_target_path_info( std::list<std::string>& device_list );
  static Addr_t find_address ( std::string from_initiator, std::string to_target );
  static void read_yaml( void );
  static void set_filename( const std::string & filename )
  {
    instance().m_yaml_filename = filename;
  }

private:
  // Gang of six
  Memory_map( void ) = default;
  ~Memory_map( void ) = default;
  Memory_map( const Memory_map& ) = delete;
  Memory_map& operator=( const Memory_map& ) = delete;
  Memory_map( const Memory_map&& ) = delete;
  Memory_map& operator=( Memory_map&& ) = delete;

  // Singleton access
  static Memory_map & instance( void );
  static Origin_map & origin_map( void );

  std::string  m_yaml_filename{ "memory_map.yaml" };
  Origin_map   m_origin_map;
};

#endif
#endif /*MEMORY_MAP_HPP*/
