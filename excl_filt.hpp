////////////////////////////////////////////////////////////////////////////////
//
//  ##### #     #  ####  #            ##### ### #     #######                     
//  #      #   #  #    # #            #      #  #        #                        
//  #       # #   #      #            #      #  #        #                        
//  #####    #    #      #            #####  #  #        #                        
//  #       # #   #      #            #      #  #        #                        
//  #      #   #  #    # #            #      #  #        #                        
//  ##### #     #  ####  ##### ###### #     ### #####    #                        
//
////////////////////////////////////////////////////////////////////////////////
#ifndef EXCL_FILT_HPP
#define EXCL_FILT_HPP

#include <tlm>
#include "common.hpp"
#include "excl_extn.hpp"

struct Excl_filt
: sc_core::sc_module
, tlm::tlm_bw_transport_if<>
, tlm::tlm_fw_transport_if<>
{
  using tlm_payload_t = tlm::tlm_generic_payload;

  // Ports
  tlm::tlm_initiator_socket<> init_socket;
  tlm::tlm_target_socket<>    targ_socket;

  Excl_filt //< Constructor
  ( sc_core::sc_module_name instance_name
  , Depth_t                 excl_locks = 1
  , Depth_t                 excl_size  = 4/*bytes*/
  , Excl_filt*              global =  nullptr;
  );
  virtual ~Excl_filt( void );
  const char* kind( void ) const override { return "Excl_filt"; }

private:
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_core::sc_time& delay );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay );
  bool get_direct_mem_ptr( tlm_payload_t& trans, tlm::tlm_dmi& dmi_data);
  unsigned int transport_dbg( tlm_payload_t& trans );

  // Backward interface
  tlm::tlm_sync_enum nb_transport_bw( tlm_payload_t& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay );
  void invalidate_direct_mem_ptr( sc_dt::uint64 start_range, sc_dt::uint64 end_range );

  // Helpers

  // Attributes
  std::vector<bool> m_excl;
  Excl_filt*        m_global{ nullptr };
};

#endif /*EXCL_FILT_HPP*/
