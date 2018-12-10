////////////////////////////////////////////////////////////////////////////////
//
//  #     #                                                                      
//  ##   ##                                                                      
//  # # # #                                                                      
//  #  #  # ### ##  #    #                                                       
//  #     # #  #  # #    #                                                       
//  #     # #  #  # #    #                                                       
//  #     # #  #  #  #####                                                       
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MMU_HPP
#define MMU_HPP
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include <tlm_utils/instance_specific_extensions.h>
#include "common.hpp"
#include "memory_manager.hpp"
#include "configuration.hpp"
#include <map>

struct Mmu_module: sc_core::sc_module
{
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_accessor_t = tlm_utils::instance_specific_extension_accessor;
  tlm_utils::multi_passthrough_target_socket<Mmu_module>    targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<Mmu_module> init_socket;

  SC_CTOR( Mmu_module );
  virtual ~Mmu_module( void );
  virtual const char* kind( void ) const { return "Mmu_module"; }

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

  // Attributes
  Memory_manager<>&        m_mm; // for managing generic payload
  tlm_accessor_t           m_accessor;
  std::map<uint64_t,Mmu_t> m_attr;
};

#endif /*MMU_HPP*/
