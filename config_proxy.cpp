////////////////////////////////////////////////////////////////////////////////
//
//   ####                                     #####                             
//  #    #                ###                 #    #                            
//  #                    #   #  #   ###       #    #                            
//  #       ####  ####  ####       #   #      #####  # ###   ####  ##  ## #   # 
//  #      #    # #   #  #     ##   ####      #      ##     #    #   ##    # #  
//  #    # #    # #   #  #      #      #      #      #      #    #   ##     #   
//   ####   ####  #   #  #     ###  ###  #### #      #       ####  ##  ##   #   
//
////////////////////////////////////////////////////////////////////////////////
#include "config_proxy.hpp"
#include "report.hpp"
namespace {
  const char* MSGID{ "/Doulos/Example/Config_proxy" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
// Constructor
Config_proxy::Config_proxy
( sc_module_name instance_name
, Depth_t        target_size
, Access         access    
, size_t         max_burst
, size_t         alignment
, uint32_t       addr_clocks
, uint32_t       read_clocks
, uint32_t       write_clocks
)
  : targ_socket{ "targ_socket" }
  , init_socket{ "init_socket" }
{
  targ_socket.bind(*this);
  init_socket.bind(*this);
  m_configuration.set( "name",         string(name())  );
  m_configuration.set( "kind",         string(kind())  );
  m_configuration.set( "object_ptr",   uintptr_t(this) );
  m_configuration.set( "target_size",  target_size     );
  m_configuration.set( "access",       access          );
  m_configuration.set( "max_burst",    max_burst       );
  m_configuration.set( "alignment",    alignment       );
  m_configuration.set( "addr_clocks",  addr_clocks     );
  m_configuration.set( "read_clocks",  read_clocks     );
  m_configuration.set( "write_clocks", write_clocks    );
  m_configuration.set( "coding_style", Style::AT       );
  INFO( ALWAYS, "Constructed " << name() << " with configuration:\n" << m_configuration );
}

//------------------------------------------------------------------------------
// Destructor
Config_proxy::~Config_proxy( void )
{
}

////////////////////////////////////////////////////////////////////////////////
// Forward interface

//------------------------------------------------------------------------------
void Config_proxy::b_transport( tlm_payload_t& trans, sc_time& delay )
{
  init_socket->b_transport( trans, delay );
}

//------------------------------------------------------------------------------
tlm_sync_enum Config_proxy::nb_transport_fw( tlm_payload_t& trans, tlm_phase& phase, sc_time& delay )
{
  return init_socket->nb_transport_fw( trans, phase, delay );
}

//------------------------------------------------------------------------------
bool Config_proxy::get_direct_mem_ptr( tlm_payload_t& trans, tlm_dmi& dmi_data )
{
  return init_socket->get_direct_mem_ptr( trans, dmi_data );
}

//------------------------------------------------------------------------------
unsigned int Config_proxy::transport_dbg( tlm_payload_t& trans )
{
  if( configure(trans) ) {
    INFO( DEBUG, "config_only" );
    trans.set_response_status( TLM_OK_RESPONSE );
    return 0;
  }
  return init_socket->transport_dbg( trans );
}

////////////////////////////////////////////////////////////////////////////////
// Backward interface

//------------------------------------------------------------------------------
tlm_sync_enum Config_proxy::nb_transport_bw( tlm_payload_t& trans, tlm_phase& phase, sc_time& delay)
{
  return targ_socket->nb_transport_bw( trans, phase, delay );
}

//------------------------------------------------------------------------------
void Config_proxy::invalidate_direct_mem_ptr( uint64 start_range , uint64 end_range )
{
  targ_socket->invalidate_direct_mem_ptr( start_range, end_range );
}

////////////////////////////////////////////////////////////////////////////////
// Helper

//------------------------------------------------------------------------------
// Return true if configuration is all that is needed
bool Config_proxy::configure ( tlm_payload_t& trans)
{
  Config_extn* extn{trans.get_extension<Config_extn>()};
  if( extn != nullptr ) {
    INFO( DEBUG, "Configuring " << name() );
    if (extn->configuration.empty()) {
      NOINFO( DEBUG, "Sending configuration:\n" << m_configuration );
      extn->configuration = m_configuration;
    }
  }
  trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
  return trans.get_command() == TLM_IGNORE_COMMAND;
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
