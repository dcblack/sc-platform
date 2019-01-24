#ifndef MEMORY_MAP_HPP
#define MEMORY_MAP_HPP
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

#include "common/common.hpp"
#include <string>
#include <ostream>
#include <map>
#include <memory>
#include <list>

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
  using Target_ptr_map = std::map< std::string, Target_info* >;
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
  static Address_map get_address_map  ( const std::string & origin_name );
  static Target_info find_target_info ( std::list<std::string>& device_list );
  static Addr_t      find_address     ( std::string from_initiator, std::string to_target );
  static int         find_irq         ( std::string target );
  static int         max_irq          ( void );
  static void        read_yaml        ( void );
  static void        set_filename     ( const std::string & filename )
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

  std::string    m_yaml_filename{ "memory_map.yaml" };
  Origin_map     m_origin_map;
  Target_ptr_map m_target_ptr_map;
  int            m_max_irq{ -1 };
};

#endif /*MEMORY_MAP_HPP*/
