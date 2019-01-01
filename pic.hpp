#ifndef PIC_MODULE_HPP
#define PIC_MODULE_HPP
////////////////////////////////////////////////////////////////////////////////
//
//  #####  ###  ####         #     # ####   #                                     
//  #    #  #  #    #        ##   ## #   #  #                                     
//  #    #  #  #             # # # # #    # #                                     
//  #####   #  #             #  #  # #    # #                                     
//  #       #  #             #     # #    # #                                     
//  #       #  #    #        #     # #   #  #                                     
//  #      ###  ####  ###### #     # ####   #####                                 
//
////////////////////////////////////////////////////////////////////////////////
#include "pic_reg.h"
#include "interrupt.hpp"
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <cstdint>
#include <vector>
#include <map>
#include <bitset>
#include <string>
#include "common.hpp"
#include "configuration.hpp"
#include "no_clock.hpp"
using sc_core::sc_time;
using sc_core::sc_event;

struct Pic_module: sc_core::sc_module
{
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_phase_t   = tlm::tlm_phase;
  using tlm_peq_t     = tlm_utils::peq_with_cb_and_phase<Pic_module>;

  // Ports
  tlm_utils::simple_target_socket<Pic_module> targ_socket    { "targ_socket"    };
  sc_core::sc_export<Interrupt_send_if>       irq_recv_xport { "irq_recv_xport" };
  sc_core::sc_port<Interrupt_send_if,0>       irq_send_port  { "irq_send_port"  };

  // Local channels
  Interrupt_channel                           irq_channel    { "irq_channel"    };

  Pic_module //< Constructor
  ( sc_core::sc_module_name instance_name
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 1
  , uint32_t                write_clocks = 1
  );
  Pic_module( Pic_module&& ) = default;
  Pic_module& operator=( Pic_module&& ) = default;
  ~Pic_module( void ); //< Destructor
  const char* kind( void ) const override { return "Pic_module"; }

private:
  //----------------------------------------------------------------------------
  // Processes
  void pic_thread( void );
  void end_of_elaboration( void ) override;

  //----------------------------------------------------------------------------
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_time& delay );
  Depth_t transport_dbg( tlm_payload_t& trans );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );
  bool get_direct_mem_ptr( tlm_payload_t& trans, tlm::tlm_dmi& dmi_data );
private:
  Pic_module( const Pic_module& ) = delete;
  Pic_module& operator=( const Pic_module& ) = delete;

  // Helpers
  void targ_peq_cb( tlm_payload_t& trans, const tlm_phase_t& phase );
  void execute_transaction( tlm_payload_t& trans );
  void send_end_req( tlm_payload_t& trans );
  void send_response( tlm_payload_t& trans );
  void execute_transaction_process( void );
  bool configure( tlm_payload_t& trans );
  bool payload_is_ok( tlm_payload_t& trans, Depth_t len, Style coding_style );
  Depth_t transport( tlm_payload_t& trans, sc_time& delay );
  //----------------------------------------------------------------------------
  // Pic Helpers
  void  write_actions( tlm_payload_t& trans, const sc_time& delay );
  void   read_actions( tlm_payload_t& trans, const sc_time& delay );

  //----------------------------------------------------------------------------
  // Attributes
  no_clock&            clk { no_clock::global( "system_clock" ) };
  Configuration        m_configuration;

  //----------------------------------------------------------------------------
  // Internal attributes
  Depth_t                     m_target_size;
  uint32_t                    m_addr_clocks;   // time to receive address/control
  uint32_t                    m_read_clocks;   // time per bus beat to respond with data
  uint32_t                    m_write_clocks;  // time per bus beat to write data
  std::vector<uint8_t>        m_register_vec { PIC_REGS_SIZE };
  tlm_peq_t                   m_targ_peq;
  Depth_t                     m_max_burst{ 4 };
  bool                        m_response_in_progress    { false };
  tlm_payload_t*              m_transaction_in_progress { nullptr };
  tlm_payload_t*              m_next_response_pending   { nullptr };
  tlm_payload_t*              m_end_req_pending         { nullptr };
  sc_event                    m_target_done_event;
  static constexpr int        M{ 32 }; // maximum # of targets for this design
  std::vector<Pic_Source_t>   m_source_irq          ; // per interrupt source
  std::vector<std::bitset<M>> m_source_targets      ; // per interrupt source
  std::vector<Pic_Target_t>   m_target_irq     { M }; // per target CPU
  std::vector<std::list<int>> m_target_level   { M }; // per target CPU
  std::vector<uint32_t>       m_target_select  { M }; // per target CPU
  std::map<std::string,int>   m_target_id           ;
  static constexpr uint32_t   m_pic_identity{ 0xD041'0501 };
};

#endif /*PIC_MODULE_HPP*/
