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
#ifndef TIMER_MODULE_H
#define TIMER_MODULE_H
#include "timer_beh.hpp"
#include "timer_reg.hpp"
#include "interrupt_if.hpp"
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <cstdint>
#include <vector>
#include <string>
#include "common.hpp"
#include "config.hpp"
#include "no_clock.hpp"
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
  , Addr_t                  target_start = 0
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 2
  , uint32_t                write_clocks = 2
  );
  Timer_module( Timer_module&& ) = default;
  Timer_module& operator=( Timer_module&& ) = default;
  ~Timer_module( void ); //< Destructor
  virtual const char* kind( void ) const { return "Timer_module"; }

private:
  //----------------------------------------------------------------------------
  // Processes
  void timer_thread( void );
  void end_of_elaboration( void );

  //----------------------------------------------------------------------------
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_core::sc_time& delay );
  Depth_t transport_dbg( tlm_payload_t& trans );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );
private:
  Timer_module( const Timer_module& ) = delete;
  Timer_module& operator=( const Timer_module& ) = delete;

  //----------------------------------------------------------------------------
  // Timer Helpers
  // Protocol Helpers
  void targ_peq_cb( tlm_payload_t& trans, const tlm_phase_t& phase );
  void execute_transaction( tlm_payload_t& trans );
  void send_end_req( tlm_payload_t& trans );
  void send_response( tlm_payload_t& trans );
  void execute_transaction_process( void );
  bool config( tlm_payload_t& trans );
  bool payload_is_ok( tlm_payload_t& trans, Depth_t len, Style coding_style );
  Depth_t transport( tlm_payload_t& trans, sc_time& delay, Depth_t len );

  //----------------------------------------------------------------------------
  // Timer Helpers
  Timer_reg& timer_reg_vec( int index );
  uint32_t scale( uint32_t status )
  {
    return ( status & TIMER_SCALE_MASK ) >> TIMER_SCALE_LSB;
  }
  bool irq_enabled( int index ) {
    return TIMER_IRQ_ENABLE == ( timer_reg_vec( index ).status & TIMER_INTERRUPT_MASK );
  }
  uint32_t get_timer_status( unsigned int index );
  void     set_timer_status( unsigned int index, const sc_time& delay );
  void  write_actions( Addr_t address, uint8_t* data_ptr, Depth_t len, const sc_time& delay );
  void   read_actions( Addr_t address, uint8_t* data_ptr, Depth_t len, const sc_time& delay );

  //----------------------------------------------------------------------------
  // Attributes
  no_clock&                 clk { no_clock::global( "system_clock" ) };
  Config                    m_config;

  //----------------------------------------------------------------------------
  // Internal attributes
  Depth_t                   m_target_depth;
  size_t                    m_timer_quantity;
  uint32_t                  m_addr_clocks;   // time to receive address/control
  uint32_t                  m_read_clocks;   // time per bus beat to respond with data
  uint32_t                  m_write_clocks;  // time per bus beat to write data
  std::vector<uint8_t>      m_register_vec;  // register storage
  sc_core::sc_vector<Timer> m_timer_vec;
  tlm_peq_t                 m_targ_peq;
  Depth_t                   m_max_burst{ 8 };
  sc_event                  m_target_done_event;
  bool                      m_response_in_progress{ false };
  tlm_payload_t*            m_transaction_in_progress{ nullptr };
  tlm_payload_t*            m_next_response_pending{ nullptr };
  tlm_payload_t*            m_end_req_pending{ nullptr };
  std::vector<sc_event>     m_timeout_event;

};

#endif /*TIMER_MODULE_H*/
