#include "exclusive/excl_filt.hpp"
////////////////////////////////////////////////////////////////////////////////
//
//  ##### #     #  ####  #            ##### ### #     #######                     
//  #      #   #  #    # #            #      #  #        #                        
//  #       # #   #      #            #      #  #        #                        
//  #####    #    #      #            #####  #  #        #                        
//  #       # #   #      #            #      #  #        #                        
//  #      #   #  #    # #            #      #  #        #                        
//  ##### #     #  ####  ##### ###### #     ### #####    #                        
//
////////////////////////////////////////////////////////////////////////////////
#include "report/report.hpp"
namespace {
  char const * const MSGID{ "/Doulos/Example/Excl_filt" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
// Constructor
Excl_filt::Excl_filt
( sc_module_name instance_name
, Depth_t        excl_size  // Max bytes per exclusive operation
, Depth_t        excl_locks // Number of locks
, Excl_filt*     global
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
Excl_filt::~Excl_filt( void )
{
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void Excl_filt::b_transport( tlm_payload_t& trans, sc_time& delay )
{
  NOT_YET_IMPLEMENTED();
  init_socket->b_transport( trans, delay );
}

//------------------------------------------------------------------------------
tlm_sync_enum Excl_filt::nb_transport_fw( tlm_payload_t& trans, tlm_phase& phase, sc_time& delay )
{
  NOT_YET_IMPLEMENTED();
  return init_socket->nb_transport_fw( trans, phase, delay );
}

//------------------------------------------------------------------------------
bool Excl_filt::get_direct_mem_ptr( tlm_payload_t& trans, tlm_dmi& dmi_data )
{
  return init_socket->get_direct_mem_ptr( trans, dmi_data );
}

//------------------------------------------------------------------------------
unsigned int Excl_filt::transport_dbg( tlm_payload_t& trans )
{
  NOT_YET_IMPLEMENTED();
  return init_socket->transport_dbg( trans );
}

////////////////////////////////////////////////////////////////////////////////
// Backward interface

//------------------------------------------------------------------------------
tlm_sync_enum Excl_filt::nb_transport_bw( tlm_payload_t& trans, tlm_phase& phase, sc_time& delay)
{
  return targ_socket->nb_transport_bw( trans, phase, delay );
}

//------------------------------------------------------------------------------
void Excl_filt::invalidate_direct_mem_ptr( uint64 start_range , uint64 end_range )
{
  targ_socket->invalidate_direct_mem_ptr( start_range, end_range );
}

////////////////////////////////////////////////////////////////////////////////
// Helper

//------------------------------------------------------------------------------
// Return true if configuration is all that is needed
bool Excl_filt::Excl ( tlm_payload_t& trans)
{
  NOT_YET_IMPLEMENTED();
  Excl_extn* extn{trans.get_extension<Excl_extn>()};
  if( extn != nullptr ) {
  }
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
