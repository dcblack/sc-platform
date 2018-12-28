////////////////////////////////////////////////////////////////////////////////
//
//  ####   #     #    #                                                          
//  #   #  ##   ##   # #                                                         
//  #    # # # # #  #   #                                                        
//  #    # #  #  # #     #                                                       
//  #    # #     # #######                                                       
//  #   #  #     # #     #                                                       
//  ####   #     # #     #                                                       
//
////////////////////////////////////////////////////////////////////////////////
#ifndef DMA_HPP
#define DMA_HPP
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

struct Dma_module: sc_core::sc_module
{
  tlm_utils::simple_initiator_socket<Dma_module> init_socket{ "init_socket" };
  tlm_utils::simple_target_socket<Dma_module>    targ_socket{ "targ_socket" };

  Dma_module
  ( sc_core::sc_module_name instance
  );
  virtual ~Dma_module( void );
  // Processes
  virtual void dma_thread( void );
  // Forward interface
  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_core::sc_time& delay );
  #if 0
  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay );
  virtual bool get_direct_mem_ptr( tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data )
  virtual unsigned int transport_dbg( tlm::tlm_generic_payload& trans );
  #endif
  #if 0
  // Backward interface
  virtual tlm::tlm_sync_enum nb_transport_bw( int id, tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay );
  virtual void invalidate_direct_mem_ptr( int id, sc_dt::uint64 start_range, sc_dt::uint64 end_range );
  #endif
private:
  // Helpers
  virtual void init_peq_cb( tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase );
  virtual void targ_peq_cb( tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase );
  virtual void transport( tlm::tlm_command cmd, uint64_t address, unsigned char* data, int32_t length );
  // Convenience
  virtual void write32  ( uint64_t address, uint32_t data )
    { write( address, static_cast<unsigned char*>(&data), 4 );
  virtual void read32   ( uint64_t address, uint32_t data )
    { read ( address, static_cast<unsigned char*>(&data), 4 );
  virtual void write16  ( uint64_t address, uint16_t data )
    { write( address, static_cast<unsigned char*>(&data), 2 );
  virtual void read16   ( uint64_t address, uint16_t data )
    { read ( address, static_cast<unsigned char*>(&data), 2 );
  virtual void write8   ( uint64_t address, uint8_t  data )
    { write( address, static_cast<unsigned char*>(&data), 1 );
  virtual void read8    ( uint64_t address, uint8_t  data )
    { read ( address, static_cast<unsigned char*>(&data), 1 );
  virtual void write    ( uint64_t address, unsigned char* data, int32_t length );
    { transport( tlm::TLM_WRITE_COMMAND, address, data, length ); }
  virtual void read     ( uint64_t address, unsigned char* data, int32_t length );
    { transport( tlm::TLM_READ_COMMAND, address, data, length ); }
  // Attributes
  tlm_utils::tlm_quantumkeeper m_qk{ "quantum_keeper" };
  tlm_utils::peq_with_cb_and_phase<Dma_module> m_init_peq;
  tlm_utils::peq_with_cb_and_phase<Dma_module> m_targ_peq;
  bool   m_use_at;
};

#endif /*DMA_HPP*/
