#ifndef TCPIP_IF_HPP
#define TCPIP_IF_HPP

#include "tcpip/tcpip_tx_if.hpp"
#include "tcpip/tcpip_rx_if.hpp"

template<typename T>
struct Tcpip_if: Tcpip_rx_if<T>, Tcpip_tx_if<T> { }

#endif/*TCPIP_IF_HPP

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-APACHE.txt.
//END tcpip_if.hpp @(#)$Id$
