////////////////////////////////////////////////////////////////////////////////
//
//   ####  ####### #    # #####                                                  
//  #    #    #    #    # #    #                                                 
//  #         #    #    # #    #                                                 
//   ####     #    #    # #####                                                  
//       #    #    #    # #    #                                                 
//  #    #    #    #    # #    #                                                 
//   ####     #     ####  #####                                                  
//
////////////////////////////////////////////////////////////////////////////////
#ifndef STUB_MODULE_HPP
#define STUB_MODULE_HPP
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <cstdint>
#include "common.hpp"
#include "configuration.hpp"
#include "no_clock.hpp"
using sc_core::sc_time;

struct Stub_module: sc_core::sc_module
{
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_phase_t   = tlm::tlm_phase;
  using tlm_peq_t     = tlm_utils::peq_with_cb_and_phase<Stub_module>;
  tlm_utils::simple_target_socket<Stub_module> targ_socket{ "targ_socket" };

  Stub_module //< Constructor
  ( sc_core::sc_module_name instance_name
  , Depth_t                 target_size  = 64/*bytes*/
  , Access                  access       = Access::none
  , size_t                  max_burst    = 8/*bytes*/
  , size_t                  alignment    = 4/*bytes*/
  , Feature                 dmi_allowed  = DMI::none
  , Feature                 byte_enables = Byte_enables::none
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 1
  , uint32_t                write_clocks = 1
  );
  Stub_module( Stub_module&& ) = default;
  Stub_module& operator=( Stub_module&& ) = default;
  ~Stub_module( void ); //< Destructor
  const char* kind( void ) const override { return "Stub_module"; }
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_time& delay );
  Depth_t transport_dbg( tlm_payload_t& trans );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );
  bool get_direct_mem_ptr( tlm_payload_t& trans, tlm::tlm_dmi& dmi_data );
private:
  Stub_module( const Stub_module& ) = delete;
  Stub_module& operator=( const Stub_module& ) = delete;

  // Helpers
  void targ_peq_cb( tlm_payload_t& trans, const tlm_phase_t& phase );
  bool payload_is_ok( tlm_payload_t& trans, Depth_t len, Style coding_style );
  Depth_t transport( tlm_payload_t& trans, sc_time& delay, Depth_t len );
  void execute_transaction( tlm_payload_t& trans );
  void send_end_req( tlm_payload_t& trans );
  void send_response( tlm_payload_t& trans );
  void execute_transaction_process( void );
  bool configure( tlm_payload_t& trans );

  // Attributes
  no_clock&            clk { no_clock::global( "system_clock" ) };
  Configuration        m_configuration;
  // Internal attributes
  Depth_t              m_target_size;
  Feature              m_dmi_allowed;
  Access               m_access;        // Default RW, optional RO
  Feature              m_byte_enables;
  size_t               m_alignment;     // required minimum alignment
  size_t               m_max_burst;     // maximum number of bytes per burst (0 => infinite for LT)
  uint32_t             m_addr_clocks;   // time to receive address/control
  uint32_t             m_read_clocks;   // time per bus beat to respond with data
  uint32_t             m_write_clocks;  // time per bus beat to write data
  tlm_peq_t            m_targ_peq;
  bool                 m_dmi_granted             { false };
  tlm_payload_t*       m_transaction_in_progress { nullptr };
  bool                 m_response_in_progress    { false };
  tlm_payload_t*       m_next_response_pending   { nullptr };
  tlm_payload_t*       m_end_req_pending         { nullptr };
  sc_core::sc_event    m_target_done_event;
};

#endif /*STUB_MODULE_HPP*/
