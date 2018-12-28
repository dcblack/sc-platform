#ifndef CPU_MODULE_HPP
#define CPU_MODULE_HPP
////////////////////////////////////////////////////////////////////////////////
//
//   ####  #####  #    #        #     # ####   #
//  #    # #    # #    #        ##   ## #   #  #
//  #      #    # #    #        # # # # #    # #
//  #      #####  #    #        #  #  # #    # #
//  #      #      #    #        #     # #    # #
//  #    # #      #    #        #     # #   #  #
//   ####  #       ####  ###### #     # ####   #####
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include "report.hpp"
#include "common.hpp"
#include "no_clock.hpp"
#include "interrupt.hpp"
#include "task.hpp"
#include "memory_manager.hpp"
#include "memory_map.hpp"
#include <map>
using sc_core::sc_time;
using sc_core::sc_event;
using sc_core::sc_signal;

struct Cpu_module: sc_core::sc_module
{
  using tlm_payload_t = tlm::tlm_generic_payload;
  using tlm_phase_t   = tlm::tlm_phase;
  using tlm_peq_t     = tlm_utils::peq_with_cb_and_phase<Cpu_module>;
  // Ports
  tlm_utils::simple_initiator_socket<Cpu_module>  init_socket { "init_socket" };
  sc_core::sc_export<Interrupt_send_if>           intrq_xport { "intrq_xport" };
  no_clock&                                       clk         { no_clock::global( "system_clock" ) };

  // Local channels
  Interrupt_channel   intrq_chan    { "intrq_signal"  };
  sc_signal<bool>     intrq_enabled { "intrq_enabled" };

  // Fundamentals
  SC_CTOR( Cpu_module );
  ~Cpu_module( void ){}
  const char* kind( void ) const override { return "Cpu_module"; }

  // Processes
  void cpu_thread( void );
  void irq_thread( void );

  // Backward interface
  tlm::tlm_sync_enum nb_transport_bw( tlm_payload_t& trans, tlm_phase_t& phase, sc_time& delay );
  void invalidate_direct_mem_ptr( Addr_t start_range, Addr_t end_range );

  // Overrides (callbacks)
  void start_of_simulation( void ) override;
  void end_of_simulation( void ) override;
private:

  // Helpers
  void init_peq_cb( tlm_payload_t& trans, const tlm_phase_t& phase );
  template<typename T>
  void transport_dbg
  ( tlm::tlm_command command
  , Addr_t           address
  , std::vector<T>&  vec
  );
  void transport
  ( tlm::tlm_command command
  , Addr_t           address
  , uint8_t*         data_ptr
  , Depth_t          data_len
  , Style            coding_style=Style::DEFAULT
  );
  void record_transaction
  ( tlm::tlm_command command
  , Addr_t           address
  , Depth_t          data_len
  , Style            coding_style=Style::DEFAULT
  );
  void check_transaction( tlm_payload_t& trans );

public:
  // Convenience
  //----------------------------------------------------------------------------
  // Obtain base address of a peripheral based on its hierarchical path name.
  Addr_t find_address( std::string path ) const;
  void write64  ( Addr_t address, uint64_t data )
    { write( address, reinterpret_cast<uint8_t*>( &data ), 8 ); }
  void read64   ( Addr_t address, uint64_t& data )
    { read ( address, reinterpret_cast<uint8_t*>( &data ), 8 ); }
  void write32  ( Addr_t address, uint32_t data )
    { write( address, reinterpret_cast<uint8_t*>( &data ), 4 ); }
  void read32   ( Addr_t address, uint32_t& data )
    { read ( address, reinterpret_cast<uint8_t*>( &data ), 4 ); }
  void write16  ( Addr_t address, uint16_t data )
    { write( address, reinterpret_cast<uint8_t*>( &data ), 2 ); }
  void read16   ( Addr_t address, uint16_t& data )
    { read ( address, reinterpret_cast<uint8_t*>( &data ), 2 ); }
  void write8   ( Addr_t address, uint8_t  data )
    { write( address, reinterpret_cast<uint8_t*>( &data ), 1 ); }
  void read8    ( Addr_t address, uint8_t&  data )
    { read ( address, reinterpret_cast<uint8_t*>( &data ), 1 ); }
  template<typename T>
  void write    ( Addr_t address, std::vector<T>& vec )
    { write( address, reinterpret_cast<uint8_t*>( vec.data() ), vec.size()*sizeof(T) ); }
  template<typename T>
  void read     ( Addr_t address, std::vector<T>& vec )
    { read ( address, reinterpret_cast<uint8_t*>( vec.data() ), vec.size()*sizeof(T) ); }
  void write    ( Addr_t address, uint8_t* const data, Depth_t len )
    { transport( tlm::TLM_WRITE_COMMAND, address, data, len ); INFO( DEBUG, "Wrote " << to_string( data, len ) << " to " << HEX << address ); }
  void read     ( Addr_t address, uint8_t* const data, Depth_t len )
    { transport( tlm::TLM_READ_COMMAND, address, data, len );  INFO( DEBUG, "Read  " << to_string( data, len ) << " fm " << HEX << address ); }

  //----------------------------------------------------------------------------
  // Transport debug conveniences
  void put( Addr_t address, std::vector<uint8_t>& vec );
  void get( Addr_t address, Depth_t depth, std::vector<uint8_t>& vec );

private:
  // Attributes
  Style                        m_coding_style{ Style::LT };
  tlm_utils::tlm_quantumkeeper m_qk;
  std::vector<tlm::tlm_dmi>    dmi_table; // Table of valid DMI regions
  Memory_manager<>&            m_mm; // for managing generic payload
  tlm_peq_t                    m_init_peq;
  tlm_payload_t*               m_request_in_progress;
  sc_event                     m_end_request_event;
  sc_event                     m_transport_done_event;
  Style                        m_prev_style;
  std::map<std::string,Addr_t> m_stat; // Statistics
  sc_core::sc_mutex            m_transport_mutex;
  Task_manager                 cpu_task_mgr; // allow multiple thread access
  static const char * const    MSGID;
};

//------------------------------------------------------------------------------
template<typename T>
void
Cpu_module::transport_dbg
( tlm::tlm_command command
, Addr_t           address
, std::vector<T>&  vec
)
{
  uint8_t* data_ptr{ reinterpret_cast<uint8_t*>( vec.data() ) };
  Depth_t  data_len{ Depth_t(vec.size()*sizeof(T)) };
  static Cpu_module::tlm_payload_t trans;
  trans.set_command        ( command  );
  trans.set_address        ( address  );
  trans.set_data_ptr       ( data_ptr );
  trans.set_data_length    ( data_len );
  trans.set_streaming_width( data_len );
  trans.set_byte_enable_ptr( nullptr  );
  trans.set_dmi_allowed    ( false    );
  trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

  int count = init_socket->transport_dbg( trans );
  if ( count != data_len ) {
    REPORT( WARNING, "Only transferred " << count << "bytes of "
                     << data_len << " requested " );
  }
  if ( trans.is_response_error() ) {
    REPORT( WARNING, "Transaction returned with error, response status = "
                   << trans.get_response_string()
    );
  }
}

#endif /*CPU_MODULE_HPP*/
