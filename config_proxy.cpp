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
  const char* MSGID{ "/Doulos/Example/TLM-bus" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
// Constructor
Config_proxy::Config_proxy
( sc_module_name instance_name
, Depth_t        target_depth
, Addr_t         target_start
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
  m_config.set( "name",         string(name())  );
  m_config.set( "kind",         string(kind())  );
  m_config.set( "object_ptr",   uintptr_t(this) );
  m_config.set( "target_start", target_start    );
  m_config.set( "target_depth", target_depth    );
  m_config.set( "access",       access          );
  m_config.set( "max_burst",    max_burst       );
  m_config.set( "alignment",    alignment       );
  m_config.set( "addr_clocks",  addr_clocks     );
  m_config.set( "read_clocks",  read_clocks     );
  m_config.set( "write_clocks", write_clocks    );
  m_config.set( "coding_style", Style::AT       );
  INFO( ALWAYS, "Constructed " << name() << " with config:\n" << m_config );
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
  if( config(trans) ) {
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
bool Config_proxy::config ( tlm_payload_t& trans)
{
  Config_extn* extn{trans.get_extension<Config_extn>()};
  if( extn != nullptr ) {
    INFO( DEBUG, "Configuring " << name() );
    if (extn->config.empty()) {
      NOINFO( DEBUG, "Sending config:\n" << m_config );
      extn->config = m_config;
    }
  }
  trans.set_gp_option( TLM_FULL_PAYLOAD_ACCEPTED );
  return trans.get_command() == TLM_IGNORE_COMMAND;
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
