#ifndef UART_MODULE_HPP
#define UART_MODULE_HPP
////////////////////////////////////////////////////////////////////////////////
//
//  #    #    #    #####  #######                                                 
//  #    #   # #   #    #    #                                                    
//  #    #  #   #  #    #    #                                                    
//  #    # #     # #####     #                                                    
//  #    # ####### #  #      #                                                    
//  #    # #     # #   #     #                                                    
//   ####  #     # #    #    #                                                    
//
////////////////////////////////////////////////////////////////////////////////
#include "uart_reg.h"
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
#include "interrupt_if.hpp"

// Specific symbols for convenience
using sc_core::sc_time;
using sc_core::sc_event;
using sc_core::sc_port;
using sc_core::SC_ZERO_OR_MORE_BOUND;

struct Uart_module: sc_core::sc_module
{
  // Type definitions to improve readability
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_phase_t   = tlm::tlm_phase;
  using tlm_peq_t     = tlm_utils::peq_with_cb_and_phase<Uart_module>;
  using Uart_txslow_port_t = sc_port<sc_core::sc_signal_out_if<bool> ,0,SC_ZERO_OR_MORE_BOUND>;
  using Uart_rxslow_port_t = sc_port<sc_core::sc_signal_in_if <bool> ,0,SC_ZERO_OR_MORE_BOUND>;
  using Uart_txfast_port_t = sc_port<sc_core::sc_fifo_out_if<uint8_t>,0,SC_ZERO_OR_MORE_BOUND>;
  using Uart_rxfast_port_t = sc_port<sc_core::sc_fifo_in_if <uint8_t>,0,SC_ZERO_OR_MORE_BOUND>;
  using Intrq_port_t  = sc_port<Interrupt_send_if,0,SC_ZERO_OR_MORE_BOUND>;

  enum class Speed { VIRTUAL, FAST, SLOW };

  // Sockets, ports and exports
  tlm_utils::simple_target_socket<Uart_module> targ_socket { "targ_socket" };
  Uart_txslow_port_t                           txslow_port { "txslow_port" };
  Uart_rxslow_port_t                           rxslow_port { "rxslow_port" };
  Uart_txfast_port_t                           txfast_port { "txfast_port" };
  Uart_rxfast_port_t                           rxfast_port { "rxfast_port" };
  Intrq_port_t                                 intrq_port  { "intrq_port"  };

  Uart_module //< Constructor
  ( sc_core::sc_module_name instance_name
  , uint32_t                addr_clocks  = 1
  , uint32_t                read_clocks  = 2
  , uint32_t                write_clocks = 3
  );
  Uart_module( Uart_module&& ) = default;
  Uart_module& operator=( Uart_module&& ) = default;
  ~Uart_module( void ); //< Destructor
  virtual const char* kind( void ) const override { return "Uart_module"; }

private:
  //----------------------------------------------------------------------------
  // Processes
  void tx_thread( void );
  void rx_thread( void );
  void end_of_elaboration( void ) override;

  //----------------------------------------------------------------------------
  // Forward interface
  void b_transport( tlm_payload_t& trans, sc_time& delay );
  Depth_t transport_dbg( tlm_payload_t& trans );
  tlm::tlm_sync_enum nb_transport_fw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );
  bool get_direct_mem_ptr( tlm_payload_t& trans, tlm::tlm_dmi& dmi_data );
private:
  Uart_module( const Uart_module& ) = delete;
  Uart_module& operator=( const Uart_module& ) = delete;

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
  // Uart Helpers
  void  write_actions( tlm_payload_t& trans, const sc_time& delay );
  void   read_actions( tlm_payload_t& trans, const sc_time& delay );
  void  send( void );

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
  std::vector<uint8_t> m_register_vec { sizeof(Uart_regs_t) };
  Uart_regs_t          m_uart;
  Speed                m_txspeed;
  Speed                m_rxspeed;
  std::deque<uint8_t>  m_txfifo;
  std::deque<uint8_t>  m_rxfifo;
  int                  m_txfifo_capacity{ 16 }; // Configured maximum
  int                  m_rxfifo_capacity{ 16 }; // Configured maximum
  Uart_ctrl_t          m_txctrl;
  Uart_ctrl_t          m_rxctrl;
  sc_event             m_txevent;
  sc_event             m_rxevent;
  uint32_t             m_txbits    { 0 };
  uint32_t             m_rxbits    { 0 };
  uint32_t             m_txcount   { 0 };
  uint32_t             m_rxcount   { 0 };
  uint32_t             m_txdropped { 0 };
  uint32_t             m_rxdropped { 0 };
  uint32_t             m_baud[15]  { 0, 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 128000, 256000 };
};

#endif /*UART_MODULE_HPP*/
