#ifndef TCPIP_TX_HPP
#define TCPIP_TX_HPP

#include "tcpip_tx_if.hpp"
#include <systemc>
#include <thread>

template<typename T>
struct Tcpip_tx_channel
: sc_core::sc_channel
, Tcpip_tx_if
{
  // Constructor
  Tcpip_tx_channel( sc_core::sc_module_name instance_name )
  {
  }

  // Destructor
  virtual ~Tcpip_tx_channel( void )
  {
  }

  void start_of_simulation()
  {
    // Launch thread
  }

  // Processes
  void tx_osthread( void ) //< not SystemC
  {
    // Create TCP/IP socket
    // Bind to incoming address
    // Listen for a connection
    // Accept connection
    // Accept connection
    for(;;) {
      // Wait for data from SystemC locally
      tx.wait_unless_available();
      // Packetize data
      // Send packet to remote
      // If shutdown, exit
    }
  }

  void end_of_simulation( void )
  {
    // Send shutdown to os thread
    // Close connection
  }

  // Methods
  virtual bool can_put( void ) const override
  {
  }

  virtual const sc_core::sc_event& ok_to_put( void ) const override
  {
  }

  virtual void put( Async_payload& the_payload ) override
  {
  }

  virtual bool nb_put( Async_payload& the_payload ) override
  {
  }

private:
  // Local methods - NONE

  // Local channels
  Async_tx_channel tx;

  // Attributes
  uint32_t    m_tx_port;
  std::thread m_tx_osthread;

  static constexpr const char* const MSGID
  { "/Doulos/{:$PROJECT:}/Tcpip_tx" };
};//end Tcpip_tx_channel

#endif/*TCPIP_TX_HPP*/

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END tcpip_tx.hpp @(#)$Id$
