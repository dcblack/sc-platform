#ifndef TCPIP_TX_IF_HPP
#define TCPIP_TX_IF_HPP

#include "async/async_payload.hpp"
#include <systemc>

struct Tcpip_tx_if
: virtual sc_core::sc_interface
{
  virtual bool can_put( void ) const = 0;
  virtual const sc_core::sc_event& ok_to_put( void ) const = 0;
  virtual void put( Async_payload& the_payload ) = 0;
  virtual bool nb_put( Async_payload& the_payload ) = 0;
};

#endif/*TCPIP_TX_IF_HPP

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END tcpip_if.hpp @(#)$Id$
