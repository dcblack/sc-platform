#ifndef TCPIP_TX_IF_HPP
#define TCPIP_TX_IF_HPP

#include "async/async_payload.hpp"
#include <tlm>

struct Tcpip_tx_if
: tlm::tlm_put_if<Async_payload>
{
};

#endif/*TCPIP_TX_IF_HPP

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-APACHE.txt.
//END tcpip_if.hpp @(#)$Id$
