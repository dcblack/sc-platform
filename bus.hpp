////////////////////////////////////////////////////////////////////////////////
//
//  #####                                                                        
//  #    #                                                                       
//  #    #         ####                                                          
//  #####  #    # #                                                              
//  #    # #    #  ####                                                          
//  #    # #    #      #                                                         
//  #####   #####  ####                                                          
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BUS_H
#define BUS_H
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include "common.hpp"
#include "memory_manager.hpp"
#include "config.hpp"
#include <map>

struct Bus_module: sc_core::sc_module
{
  using tlm_payload_t = tlm::tlm_generic_payload;
  tlm_utils::multi_passthrough_target_socket<Bus_module>    targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<Bus_module> init_socket;

  SC_CTOR( Bus_module );
  virtual ~Bus_module( void );
  virtual const char* kind() const { return "Bus_module"; }

  // Forward interface
  void b_transport( int id, tlm_payload_t& trans, sc_core::sc_time& delay );
  tlm::tlm_sync_enum nb_transport_fw( int id, tlm_payload_t& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay );
  bool get_direct_mem_ptr( int id, tlm_payload_t& trans, tlm::tlm_dmi& dmi_data);
  unsigned int transport_dbg( int id, tlm_payload_t& trans );

  // Backward interface
  tlm::tlm_sync_enum nb_transport_bw( int id, tlm_payload_t& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay );
  void invalidate_direct_mem_ptr( int id, sc_dt::uint64 start_range, sc_dt::uint64 end_range );

private:
  // Helpers
  unsigned int decode_address( uint64_t address, uint64_t& masked_address );
  uint64_t reconstruct_address( uint64_t address, int id );
  bool mask_if_fits( Addr_t& address, Addr_t start, Depth_t depth ) const;
  bool config( tlm_payload_t& trans );
  void build_port_map( void );
  void check_port_map( void );
  void dump_port_map( int level );

  // Attributes
  struct Socket_info {
    int         port  { -1       };
    Addr_t      start { MAX_ADDR };
    Depth_t     depth { 0        };
    std::string name  { ""       }; 
    std::string kind  { ""       }; 
  };
  Config                                  m_config;
  std::vector<Socket_info>                m_port_vec;
  std::map<tlm_payload_t*, unsigned int>  m_id_map;
  Memory_manager<>&                       m_mm; // for managing generic payload
};

#endif /*BUS_H*/
