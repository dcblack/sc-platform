#include "mmu/mmu.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//  #     #                                                                      
//  ##   ##                                                                      
//  # # # #                                                                      
//  #  #  # ### ##  #    #                                                       
//  #     # #  #  # #    #                                                       
//  #     # #  #  # #    #                                                       
//  #     # #  #  #  #####                                                       
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
#include "mmu/mmu_extn.hpp"
#include <memory>
namespace {
  char const * const MSGID{ "/Doulos/Example/TLM-mmu" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

#include <tlm_utils/simple_target_socket.h>

//------------------------------------------------------------------------------
// Constructor
Mmu_module::Mmu_module( sc_module_name instance_name )
  : targ_socket{ "targ_socket" }
  , init_socket{ "init_socket" }
  , m_mm       { Memory_manager<>::instance() }
{
  targ_socket.register_b_transport( this, &Mmu_module::b_transport );
  targ_socket.register_nb_transport_fw( this, &Mmu_module::nb_transport_fw );
  targ_socket.register_get_direct_mem_ptr( this, &Mmu_module::get_direct_mem_ptr );
  targ_socket.register_transport_dbg( this, &Mmu_module::transport_dbg );
  init_socket.register_nb_transport_bw( this, &Mmu_module::nb_transport_bw );
  init_socket.register_invalidate_direct_mem_ptr( this, &Mmu_module::invalidate_direct_mem_ptr );
}

//------------------------------------------------------------------------------
// Destructor
Mmu_module::~Mmu_module( void )
{
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void
Mmu_module::b_transport( int id, tlm_payload_t& trans, sc_time& delay )
{
  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );

  if( target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    return;
  }

  INFO( DEBUG+1, name() << " forwarding to port " << target << " with address " << masked_address );
  trans.set_address( masked_address );
  init_socket[target]->b_transport( trans, delay );
}

//------------------------------------------------------------------------------
tlm_sync_enum
Mmu_module::nb_transport_fw
( int id
  , tlm_payload_t& trans
  , tlm_phase& phase
  , sc_time& delay
)
{
  Route_extn* route_extn;
  if(phase == tlm::BEGIN_REQ)
  {
    route_extn = new Route_extn;
    route_extn->id = id;
    m_accessor(trans).set_extension(route_extn);
  }

  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );

  if( target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    if( route_extn != nullptr ) {
      m_accessor(trans).clear_extension(route_extn);
      delete route_extn;
    }
    return TLM_COMPLETED;
  }

  trans.set_address( masked_address );

  tlm_sync_enum status = init_socket[target]->nb_transport_fw( trans, phase, delay );

  if(status == tlm::TLM_COMPLETED and route_extn != nullptr) {
    m_accessor(trans).clear_extension(route_extn);
    delete route_extn;
  }

  return status;
}

//------------------------------------------------------------------------------
bool
Mmu_module::get_direct_mem_ptr
( int id
  , tlm_payload_t& trans
  , tlm_dmi& dmi_data
)
{
  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );

  if( target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    return false;
  }

  trans.set_address( masked_address );

  bool status = init_socket[target]->get_direct_mem_ptr( trans, dmi_data );

  dmi_data.set_start_address( reconstruct_address( dmi_data.get_start_address(), id ) );
  dmi_data.set_end_address( reconstruct_address( dmi_data.get_end_address(),   id ) );

  return status;
}

//------------------------------------------------------------------------------
unsigned int
Mmu_module::transport_dbg( int id, tlm_payload_t& trans )
{
  uint64 address{ trans.get_address() };

  uint64 masked_address;
  unsigned int target = decode_address( address, masked_address );

  if( target > init_socket.size() ) {
    if( g_error_at_target ) {
      REPORT( ERROR, "Out of range on device " << name() << " with address " << trans.get_address() );
      trans.set_response_status( TLM_OK_RESPONSE );
    } else {
      trans.set_response_status( TLM_ADDRESS_ERROR_RESPONSE );
    }
    return 0;
  }

  trans.set_address( masked_address );

  return init_socket[target]->transport_dbg( trans );
}

////////////////////////////////////////////////////////////////////////////////
// Backward interface

//------------------------------------------------------------------------------
tlm_sync_enum
Mmu_module::nb_transport_bw
( int id
, tlm_payload_t& trans
, tlm_phase&     phase
, sc_time&       delay
)
{
  Route_extn* route_extn;
  m_accessor( trans ).get_extension( route_extn );
  sc_assert( route_extn != nullptr );
  tlm::tlm_sync_enum status = targ_socket[ route_extn->id ]->nb_transport_bw( trans, phase, delay );
  if( status == tlm::TLM_COMPLETED )
  {
    m_accessor( trans ).clear_extension( route_extn );
    delete route_extn;
  }
  return status;
}

//------------------------------------------------------------------------------
void
Mmu_module::invalidate_direct_mem_ptr
( int id
  , uint64 start_range
  , uint64 end_range
)
{
  // Reconstruct address range in system memory map
  sc_dt::uint64 bw_start_range = reconstruct_address( start_range, id );
  sc_dt::uint64 bw_end_range   = reconstruct_address( end_range,   id );

  // Propagate call backward to all initiators
  for( unsigned int i = 0; i < targ_socket.size(); i++ ) {
    targ_socket[i]->invalidate_direct_mem_ptr( bw_start_range, bw_end_range );
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helpers

//------------------------------------------------------------------------------
unsigned int //< port index
Mmu_module::decode_address
( uint64_t address,
  uint64_t& masked_address
)
{
  INFO( DEBUG+1, "Decoding address " << HEX << address );
  masked_address = address;
  unsigned int port = 0;

  NOT_YET_IMPLEMENTED();

  return port;
#endif
}

//------------------------------------------------------------------------------
uint64_t // address
Mmu_module::reconstruct_address
( uint64_t address
  , int id
)
{
  Addr_t  reconstructed{ address };
  NOT_YET_IMPLEMENTED();

  return reconstructed;
#else

  // Crude
  if( id == ROM_PORT ) {
    return ROM_BASE + address;
  }

  if( id == RAM_PORT ) {
    return RAM_BASE + address;
  }

#endif
  return MAX_ADDR;
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
