#ifndef USART_MODULE_HPP
#define USART_MODULE_HPP
#include "usart_reg.h"
//{:Other local includes needed by THIS FILE here -- DELETE THIS LINE:}
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <cstdint>
#include <vector>
#include <string>
//{:Other library includes needed by THIS FILE here -- DELETE THIS LINE:}
#include "common.hpp"
#include "configuration.hpp"
#include "no_clock.hpp"

// Specific symbols for convenience
using sc_core::sc_time;
using sc_core::sc_event;

struct Usart_module: sc_core::sc_module
{
  // Type definitions to improve readability
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_phase_t   = tlm::tlm_phase;
  using tlm_peq_t     = tlm_utils::peq_with_cb_and_phase<Usart_module>;

  // Sockets, ports and exports
  tlm_utils::simple_target_socket<Usart_module> targ_socket{ "targ_socket" };

  Usart_module //< Constructor
  ( sc_core::sc_module_name instance_name
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 2
  , uint32_t                write_clocks = 3
  );
  Usart_module( Usart_module&& ) = default;
  Usart_module& operator=( Usart_module&& ) = default;
  ~Usart_module( void ); //< Destructor
  virtual const char* kind( void ) const override { return "Usart_module"; }

private:
  //----------------------------------------------------------------------------
  // Processes
  void usart_thread( void );
  void end_of_elaboration( void ) override;

  //----------------------------------------------------------------------------
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_time& delay );
  Depth_t transport_dbg( tlm_payload_t& trans );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );
  bool get_direct_mem_ptr( tlm_payload_t& trans, tlm::tlm_dmi& dmi_data );
private:
  Usart_module( const Usart_module& ) = delete;
  Usart_module& operator=( const Usart_module& ) = delete;

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
  // Usart Helpers
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
  std::vector<uint8_t>        m_register_vec { USART_REGS_SIZE };
  tlm_peq_t            m_targ_peq;
  size_t               m_max_burst{ 4 };// maximum number of bytes per burst (0 => infinite for LT)
  bool                 m_response_in_progress    { false };
  tlm_payload_t*       m_transaction_in_progress { nullptr };
  tlm_payload_t*       m_next_response_pending   { nullptr };
  tlm_payload_t*       m_end_req_pending         { nullptr };
  sc_event             m_target_done_event;
  // {:ADDITIONAL ATTRIBUTES AS NEEDED:}
};

#endif /*USART_MODULE_HPP*/
