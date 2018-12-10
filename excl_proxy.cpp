////////////////////////////////////////////////////////////////////////////////
//
//  #####                                                                        
//  #                   ##                                                       
//  #                    #                                                       
//  ##### ##  ##  ####   #          ####  # ###   ####  ##  ## #   #             
//  #       ##   #       #          #   # ##     #    #   ##    # #              
//  #       ##   #       #          ####  #      #    #   ##     #               
//  ##### ##  ##  ####  ###  ###### #     #       ####  ##  ##   #               
//
////////////////////////////////////////////////////////////////////////////////
#include "Excl_proxy.hpp"
#include "report.hpp"
namespace {
  const char* MSGID{ "/Doulos/Example/Excl_proxy" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
// Constructor
Excl_proxy::Excl_proxy
( sc_module_name instance_name
, Depth_t        excl_size  // Max bytes per exclusive operation
, Depth_t        excl_locks // Number of locks
, Excl_proxy*    global
)
  : targ_socket{ "targ_socket" }
  , init_socket{ "init_socket" }
  , m_excl_locks( excl_locks   )
  , m_excl_size ( excl_size    )
{
  sc_assert( excl_locks > 0 and excl_size > 0 );
  excl_depth.resize( excl_locks );
  targ_socket.bind(*this);
  init_socket.bind(*this);
  INFO( ALWAYS, "Constructed " << name() << " with configuration:\n" << m_configuration );
}

//------------------------------------------------------------------------------
// Destructor
Excl_proxy::~Excl_proxy( void )
{
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void Excl_proxy::b_transport( tlm_payload_t& trans, sc_time& delay )
{
  init_socket->b_transport( trans, delay );
}

//------------------------------------------------------------------------------
tlm_sync_enum Excl_proxy::nb_transport_fw( tlm_payload_t& trans, tlm_phase& phase, sc_time& delay )
{
  return init_socket->nb_transport_fw( trans, phase, delay );
}

//------------------------------------------------------------------------------
bool Excl_proxy::get_direct_mem_ptr( tlm_payload_t& trans, tlm_dmi& dmi_data )
{
  return init_socket->get_direct_mem_ptr( trans, dmi_data );
}

//------------------------------------------------------------------------------
unsigned int Excl_proxy::transport_dbg( tlm_payload_t& trans )
{
  return init_socket->transport_dbg( trans );
}

////////////////////////////////////////////////////////////////////////////////
// Backward interface

//------------------------------------------------------------------------------
tlm_sync_enum Excl_proxy::nb_transport_bw( tlm_payload_t& trans, tlm_phase& phase, sc_time& delay)
{
  return targ_socket->nb_transport_bw( trans, phase, delay );
}

//------------------------------------------------------------------------------
void Excl_proxy::invalidate_direct_mem_ptr( uint64 start_range , uint64 end_range )
{
  targ_socket->invalidate_direct_mem_ptr( start_range, end_range );
}

////////////////////////////////////////////////////////////////////////////////
// Helper

//------------------------------------------------------------------------------
// Return true if configuration is all that is needed
bool Excl_proxy::Excl ( tlm_payload_t& trans)
{
  Excl_extn* extn{trans.get_extension<Excl_extn>()};
  if( extn != nullptr ) {
  }
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
