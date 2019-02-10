#ifndef TCPIP_RX_IF_HPP
#define TCPIP_RX_IF_HPP

#include "async/async_payload.hpp"
#include <tlm>

struct Tcpip_rx_if
: tlm::tlm_get_if<Async_payload>
{
};

#endif/*TCPIP_RX_IF_HPP

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END tcpip_if.hpp @(#)$Id$
