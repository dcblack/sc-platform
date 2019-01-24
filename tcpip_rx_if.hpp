#ifndef TCPIP_RX_IF_HPP
#define TCPIP_RX_IF_HPP

#include "async_payload.hpp"
#include <systemc>

struct Tcpip_rx_if
: virtual sc_core::sc_interface
{
  virtual bool can_get() = 0;
  virtual const sc_core::sc_event& ok_to_get() = 0;
  virtual void get( Async_payload& the_payload ) = 0;
  virtual bool nb_get( Async_payload& the_payload ) = 0;
};

#endif/*TCPIP_RX_IF_HPP

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END tcpip_if.hpp @(#)$Id$
