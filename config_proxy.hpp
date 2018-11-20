////////////////////////////////////////////////////////////////////////////////
//
//   ####                                     #####                             
//  #    #                ###                 #    #                            
//  #                    #   #  #   ###       #    #                            
//  #       ####  ####  ####       #   #      #####  # ###   ####  ##  ## #   # 
//  #      #    # #   #  #     ##   ####      #      ##     #    #   ##    # #  
//  #    # #    # #   #  #      #      #      #      #      #    #   ##     #   
//   ####   ####  #   #  #     ###  ###  #### #      #       ####  ##  ##   #   
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CONFIG_PROXY_HPP
#define CONFIG_PROXY_HPP

#include <tlm>
#include "common.hpp"
#include "config_extn.hpp"

struct Config_proxy
: sc_core::sc_module
, tlm::tlm_bw_transport_if<>
, tlm::tlm_fw_transport_if<>
{
  using tlm_payload_t = tlm::tlm_generic_payload;

  // Ports
  tlm::tlm_initiator_socket<> init_socket;
  tlm::tlm_target_socket<>    targ_socket;

  Config_proxy //< Constructor
  ( sc_core::sc_module_name instance_name
  , Depth_t                 target_depth = 0/*bytes*/
  , Addr_t                  target_start = 0
  , Access                  access       = Access::none
  , size_t                  max_burst    = 8/*bytes*/
  , size_t                  alignment    = 4/*bytes*/
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 2
  , uint32_t                write_clocks = 3
  );
  virtual ~Config_proxy( void );
  virtual const char* kind( void ) const { return "Config_proxy"; }

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
  bool config( tlm_payload_t& trans );

  // Attributes
  Config       m_config;
  Config_extn* m_extn{nullptr};
};

#endif /*CONFIG_PROXY_HPP*/
