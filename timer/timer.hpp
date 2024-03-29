#ifndef TIMER_MODULE_H
#define TIMER_MODULE_H
////////////////////////////////////////////////////////////////////////////////
//
//  ####### ### #     # ##### #####         #     # ####   #                     
//     #     #  ##   ## #     #    #        ##   ## #   #  #                     
//     #     #  # # # # #     #    #        # # # # #    # #                     
//     #     #  #  #  # ##### #####         #  #  # #    # #                     
//     #     #  #     # #     #  #          #     # #    # #                     
//     #     #  #     # #     #   #         #     # #   #  #                     
//     #    ### #     # ##### #    # ###### #     # ####   #####                 
//
////////////////////////////////////////////////////////////////////////////////
#include "timer/timer_beh.hpp"
#include "timer/timer_reg.h"
#include "interrupt/interrupt_if.hpp"
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <cstdint>
#include <vector>
#include <string>
#include "common/common.hpp"
#include "config/configuration.hpp"
#include "no_clock/no_clock.hpp"
using sc_core::sc_time;
using sc_core::sc_event;

struct Timer_module: sc_core::sc_module
{

  //----------------------------------------------------------------------------
  // Types
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_phase_t   = tlm::tlm_phase;
  using tlm_peq_t     = tlm_utils::peq_with_cb_and_phase<Timer_module>;

  //----------------------------------------------------------------------------
  // Ports
  tlm_utils::simple_target_socket<Timer_module>                             targ_socket{ "targ_socket" };
  sc_core::sc_port<Interrupt_send_if,0,sc_core::SC_ZERO_OR_MORE_BOUND>      intrq_port { "intrq_port"  };
  sc_core::sc_port<sc_core::sc_signal_out_if<bool>,0,sc_core::SC_ZERO_OR_MORE_BOUND> pulse_port { "pulse_port"  };

  //----------------------------------------------------------------------------
  // Fundamentals
  Timer_module //< Constructor
  ( sc_core::sc_module_name instance_name
  , size_t                  timer_quantity
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 2
  , uint32_t                write_clocks = 2
  );
  ~Timer_module( void ); //< Destructor
  const char* kind( void ) const override { return "Timer_module"; }

private:
  //----------------------------------------------------------------------------
  // Processes
  void timer_thread( void );
  void end_of_elaboration( void ) override;

  //----------------------------------------------------------------------------
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_time& delay );
  Depth_t transport_dbg( tlm_payload_t& trans );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );

  //----------------------------------------------------------------------------
  // Timer Helpers
  // Protocol Helpers
  void targ_peq_cb( tlm_payload_t& trans, const tlm_phase_t& phase );
  void execute_transaction( tlm_payload_t& trans );
  void send_end_req( tlm_payload_t& trans );
  void send_response( tlm_payload_t& trans );
  void execute_transaction_process( void );
  bool configure( tlm_payload_t& trans );
  bool payload_is_ok( tlm_payload_t& trans, Depth_t len, Style coding_style );
  Depth_t transport( tlm_payload_t& trans, sc_time& delay, Depth_t len );

  //----------------------------------------------------------------------------
  // Timer Helpers
  Timer_reg_t& timer_reg_vec( size_t index );
  uint32_t scale( uint32_t status )
  {
    return (( status & TIMER_SCALE_MASK ) >> TIMER_SCALE_LSB) + 1;
  }
  bool irq_enabled( int index ) {
    return TIMER_IRQ_ENABLE == ( timer_reg_vec( index ).status & TIMER_INTERRUPT_MASK );
  }
  uint32_t get_timer_status( size_t index );
  void     set_timer_status( size_t index, const sc_time& delay );
  void  write_actions( Addr_t address, uint8_t* data_ptr, Depth_t len, const sc_time& delay );
  void   read_actions( Addr_t address, uint8_t* data_ptr, Depth_t len, const sc_time& delay );

  //----------------------------------------------------------------------------
  // Attributes
  no_clock&                 clk { no_clock::global( "system_clock" ) };
  Configuration             m_configuration;

  //----------------------------------------------------------------------------
  // Internal attributes
  Depth_t                   m_target_size;
  size_t                    m_timer_quantity;
  uint32_t                  m_addr_clocks;   // time to receive address/control
  uint32_t                  m_read_clocks;   // time per bus beat to respond with data
  uint32_t                  m_write_clocks;  // time per bus beat to write data
  std::vector<uint8_t>      m_register_vec;  // register storage
  sc_core::sc_vector<Timer> m_timer_vec;
  tlm_peq_t                 m_targ_peq;
  Depth_t                   m_max_burst               { 8 };
  bool                      m_response_in_progress    { false };
  tlm_payload_t*            m_transaction_in_progress { nullptr };
  tlm_payload_t*            m_next_response_pending   { nullptr };
  tlm_payload_t*            m_end_req_pending         { nullptr };
  sc_event                  m_target_done_event;
  std::vector<sc_event>     m_timeout_event;

};

#endif /*TIMER_MODULE_H*/
