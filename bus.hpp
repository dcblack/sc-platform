////////////////////////////////////////////////////////////////////////////////
//
//  #####  #    #  ####         #     # ####   #
//  #    # #    # #    #        ##   ## #   #  #
//  #    # #    # #             # # # # #    # #
//  #####  #    #  ####         #  #  # #    # #
//  #    # #    #      #        #     # #    # #
//  #    # #    # #    #        #     # #   #  #
//  #####   ####   ####  ###### #     # ####   #####
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BUS_MODULE_HPP
#define BUS_MODULE_HPP
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include <tlm_utils/instance_specific_extensions.h>
#include "common.hpp"
#include "memory_manager.hpp"
#include "memory_map.hpp"
#include "configuration.hpp"
#include <map>
#include <functional>

struct Bus_module: sc_core::sc_module
{
  // Local types
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_accessor_t = tlm_utils::instance_specific_extension_accessor;

  // Ports
  tlm_utils::multi_passthrough_target_socket<Bus_module>    targ_socket { "targ_socket" };
  tlm_utils::multi_passthrough_initiator_socket<Bus_module> init_socket { "init_socket" };

  // Basics
  SC_CTOR( Bus_module );
  virtual ~Bus_module( void );
  virtual const char* kind( void ) const { return "Bus_module"; }
  void start_of_simulation( void );

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
  unsigned int decode_address( Addr_t address, Addr_t& masked_address );
  uint64_t reconstruct_address( Addr_t address, int id, bool bias_upwards );
  bool mask_if_fits( Addr_t& address, Addr_t start, Depth_t depth ) const;
  bool configure( tlm_payload_t& trans );
  void build_port_map( void );
  void check_port_map_and_update_configuration( void );
  void dump_port_map( int level );
  // Attributes
  Configuration     m_configuration;
  Address_map       m_addr_map; //< address => target info
  Memory_manager<>& m_mm; // for managing generic payload
  tlm_accessor_t    m_accessor;
};

#endif /*BUS_MODULE_HPP*/
