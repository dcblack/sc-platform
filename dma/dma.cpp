#include "dma/dma.hpp"
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
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
// Constructor
Dma_module::Dma_module
( sc_module_name instance_name
)
: m_init_peq( "init_peq", this, &Dma_module::init_peq_cb )
, m_targ_peq( "targ_peq", this, &Dma_module::targ_peq_cb )
{
//init_socket.register_nb_transport_bw           ( this, &Dma_module::nb_transport_bw );
//init_socket.register_invalidate_direct_mem_ptr ( this, &Dma_module::invalidate_direct_mem_ptr );
  m_qk.reset();
}

//------------------------------------------------------------------------------
// Destructor
Dma_module::~Dma_module( void )
{
}

////////////////////////////////////////////////////////////////////////////////
// Backward interface

//------------------------------------------------------------------------------
tlm_sync_enum
Dma_module::nb_transport_bw
( int id
, tlm_generic_payload& trans
, tlm_phase& phase
, sc_time& delay
)
{
  //{:To_be_supplied:}
}

//------------------------------------------------------------------------------
void
Dma_module::invalidate_direct_mem_ptr
( int id
, uint64 start_range
, uint64 end_range
)
{
  //{:To_be_supplied:}
}

////////////////////////////////////////////////////////////////////////////////
// Processes

//------------------------------------------------------------------------------
void
Dma_module::dma_thread( void )
{
  //{:To_be_supplied:}
}

////////////////////////////////////////////////////////////////////////////////
// Helpers

//------------------------------------------------------------------------------
void
Dma_module::init_peq_cb
( tlm::tlm_generic_payload& trans
, const tlm::tlm_phase& phase
)
{
  //{:To_be_supplied:}
}

//------------------------------------------------------------------------------
void
Dma_module::targ_peq_cb
( tlm::tlm_generic_payload& trans
, const tlm::tlm_phase& phase
)
{
  //{:To_be_supplied:}
}

//------------------------------------------------------------------------------
void
Dma_module::transport
( tlm::tlm_command cmd
, uint64_t address
, unsigned char* data
, int32_t length
)
// Determines AT/LT and burst size
{
  //{:To_be_supplied:}
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
