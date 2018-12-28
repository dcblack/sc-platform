#ifndef GPIO_MODULE_HPP
#define GPIO_MODULE_HPP
#include "gpio_reg.h"
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
#include "configuration.hpp"
#include "no_clock.hpp"
#include "interrupt.hpp"

// Specific symbols for convenience
using sc_core::sc_time;
using sc_core::sc_event;
using sc_core::sc_vector;
using sc_core::sc_export;
using sc_core::sc_signal_inout_if;

struct Gpio_module: sc_core::sc_module
{
  static constexpr int const PINS { 64 };

  // Type definitions to improve readability
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_phase_t   = tlm::tlm_phase;
  using tlm_peq_t     = tlm_utils::peq_with_cb_and_phase<Gpio_module>;
  using Gpio_signal_t = sc_core::sc_signal_resolved;
  using Gpio_export_t = sc_export<sc_signal_inout_if<sc_dt::sc_logic>>;
  using Intrq_port_t  = sc_core::sc_port<Interrupt_send_if,0,sc_core::SC_ZERO_OR_MORE_BOUND>;

  // Sockets, ports and exports
  tlm_utils::simple_target_socket<Gpio_module> targ_socket { "targ_socket" };
  sc_vector<Gpio_export_t>                     gpio_xport  { "gpio_export", PINS };
  Intrq_port_t                                 intrq_port  { "intrq_port"  };

  Gpio_module //< Constructor
  ( sc_core::sc_module_name instance_name
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 2
  , uint32_t                write_clocks = 3
  );
  Gpio_module( Gpio_module&& ) = default;
  Gpio_module& operator=( Gpio_module&& ) = default;
  ~Gpio_module( void ); //< Destructor
  virtual const char* kind( void ) const override { return "Gpio_module"; }

private:
  //----------------------------------------------------------------------------
  // Processes
  void end_of_elaboration( void ) override;
  void start_of_simulation( void ) override;
  void gpio_thread( void );

  //----------------------------------------------------------------------------
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_time& delay );
  Depth_t transport_dbg( tlm_payload_t& trans );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );
  bool get_direct_mem_ptr( tlm_payload_t& trans, tlm::tlm_dmi& dmi_data );
private:
  Gpio_module( const Gpio_module& ) = delete;
  Gpio_module& operator=( const Gpio_module& ) = delete;

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
  // Gpio Helpers
  void  write_actions( tlm_payload_t& trans, const sc_time& delay );
  void   read_actions( tlm_payload_t& trans, const sc_time& delay );

  //----------------------------------------------------------------------------
  // Attributes
  no_clock&            clk { no_clock::global( "system_clock" ) };
  Configuration        m_configuration;

  //----------------------------------------------------------------------------
  // Internal attributes
  Depth_t              m_target_size;
  uint32_t             m_addr_clocks;   // time to receive address/control
  uint32_t             m_read_clocks;   // time per bus beat to respond with data
  uint32_t             m_write_clocks;  // time per bus beat to write data
  tlm_peq_t            m_targ_peq;
  size_t               m_max_burst{ 4 };// maximum number of bytes per burst (0 => infinite for LT)
  bool                 m_response_in_progress    { false };
  tlm_payload_t*       m_transaction_in_progress { nullptr };
  tlm_payload_t*       m_next_response_pending   { nullptr };
  tlm_payload_t*       m_end_req_pending         { nullptr };
  sc_event             m_target_done_event;
  std::vector<uint8_t> m_register_vec { GPIO_REGS_SIZE };
  Gpio_regs_t          m_reg;
  sc_dt::sc_lv< PINS > m_out;
  sc_dt::sc_lv< PINS > m_inp;

  //----------------------------------------------------------------------------
  // Local channels
  sc_vector<Gpio_signal_t> gpio_sig{ "gpio_sig", PINS };

};

#endif /*GPIO_MODULE_HPP*/
