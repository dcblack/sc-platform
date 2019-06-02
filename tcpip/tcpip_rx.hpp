#ifndef TCPIP_RX_HPP
#define TCPIP_RX_HPP

#include "tcpip/tcpip_rx_if.hpp"
#include <systemc>
#include <thread>

template<typename T>
struct Tcpip_rx_channel
: sc_core::sc_channel
, Tcpip_rx_if
{
  // Constructor
  Tcpip_rx_channel( sc_core::sc_module_name instance_name )
  {
  }

  // Destructor
  virtual ~Tcpip_rx_channel( void )
  {
  }

  void start_of_simulation()
  {
    // Launch thread
  }

  // Processes
  void rx_osthread( void ) //< not SystemC
  {
    // Create TCP/IP socket
    // Bind to incoming address
    // Listen for a connection
    // Accept connection
    // Accept connection
    for(;;) {
      // Wait for data remotely
      rx.wait_unless_available();
      // Packetize data
      // Send packet to SystemC locally
      // If shutdown, exit
    }
  }

  void end_of_simulation( void )
  {
    // Send shutdown to os thread
    // Close connection
  }

private:
  // Local methods - NONE

  // Local channels
  Async_rx_channel tx;

  // Attributes
  uint32_t    m_rx_port;
  std::thread m_rx_osthread;

  static constexpr const char* const MSGID { "/Doulos/Example/Tcpip_rx" };
};//end Tcpip_rx_channel

#endif/*TCPIP_RX_HPP*/

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-APACHE.txt.
//END tcpip_rx.hpp @(#)$Id$
