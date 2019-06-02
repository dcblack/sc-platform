#ifndef TCPIP_TX_HPP
#define TCPIP_TX_HPP

#include "tcpip/tcpip_tx_if.hpp"
#include <systemc>
#include <thread>
#include <strings.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

template<typename T>
struct Tcpip_tx_channel
: sc_core::sc_channel
, Tcpip_tx_if
{
  // Constructor
  Tcpip_tx_channel( sc_core::sc_module_name instance_name, uint32_t port )
  : m_tx_port( port )
  {
  }

  // Destructor
  virtual ~Tcpip_tx_channel( void )
  {
  }

  void start_of_simulation()
  {
    // Launch thread
    m_tx_osthread = std::thread( tx_osthread );
  }

  // Processes
  void tx_osthread( void ) //< not SystemC
  {
    //--------------------------------------------------------------------------
    // Create TCP/IP socket
    //--------------------------------------------------------------------------
    int listening_socket = socket( AF_INET, SOCK_STREAM, 0 );
    if ( listening_socket == -1 ) {
      REPORT( FATAL, "Could not create socket" );
    }

    int option_value = 1;
    if ( setsockopt
         ( listening_socket
         , SOL_SOCKET
         , SO_REUSEADDR
         , ( void* )&option_value
         , ( socklen_t )( sizeof( socklen_t ) )
         ) < 0
       )
    {
      REPORT( FATAL, "Unable to set socket option" );
    }

    //--------------------------------------------------------------------------
    // Prepare the server address sockaddr_in structure
    //--------------------------------------------------------------------------
    struct sockaddr_in local_server;
    bzero( ( char* ) &local_server, sizeof( local_server ) );
    local_server.sin_family = AF_INET;
    local_server.sin_addr.s_addr = INADDR_ANY;
    local_server.sin_port = htons( m_tcpip_port ); // Host to Network -- big-endian conversion

    //--------------------------------------------------------------------------
    // Bind to incoming address in preparation to listening
    //--------------------------------------------------------------------------
    if ( 0 != ::bind
                ( listening_socket
                , ( struct sockaddr * )&local_server
                , sizeof( local_server )
                )
       )
    {
      REPORT( FATAL, "Bind failed" );
      sc_core::sc_abort();
    }
    REPORT( NOTE, "Bind done" );

    accept_loop: for(;;) {
      //------------------------------------------------------------------------
      // Listen for a connection
      //------------------------------------------------------------------------
      REPORT( INFO, "Queueing incoming connections..." );
      listen( listening_socket, 1 /*request at a time*/ );

      //------------------------------------------------------------------------
      // Accept connection
      //------------------------------------------------------------------------
      REPORT( INFO, "Waiting for incoming connections..." );
      // Accept an incoming connection
      int addr_len = sizeof( struct sockaddr_in );
      struct sockaddr_in remote_client;
      bzero( ( char* ) &remote_client, sizeof( local_server ) );
      incoming_socket = accept( listening_socket
                              , ( struct sockaddr * )&remote_client
                              , ( socklen_t * )&addr_len
                              );
      if ( incoming_socket < 0 ) {
        REPORT( FATAL, "Accept failed: " << strerror( errno ) );
      }
      REPORT( NOTE, "Connection accepted..." );
      //------------------------------------------------------------------------
      // Check for valid requestor
      //------------------------------------------------------------------------
      {
        std::unique_ptr<char*> recv_buffer{ new char[256] };
        bzero( recv_buffer.get(), 256 );
        n = read( sockfd, recv_buffer.get(), 255 );
        if ( n < 0 ) {
          REPORT( ERROR, "ERROR reading from socket" );
        }
        if ( SECRET != recv_buffer.get() ) {
          continue;
        }
        REPORT( NOTE, "Password accepted..." );
      }

      //------------------------------------------------------------------------
      // Begin receiving and transmitting data
      //------------------------------------------------------------------------
      server_loop: for(;;) {
        //----------------------------------------------------------------------
        // Wait for data from SystemC locally
        //----------------------------------------------------------------------
        tx.wait_unless_available();

        //----------------------------------------------------------------------
        // Exit if commanded
        //----------------------------------------------------------------------
        if ( tlmx_trans_ptr->command == TLMX_EXIT ) {
          REPORT( NOTE, "Exiting due to TLMX_EXIT..." );
          break;
        }

        //----------------------------------------------------------------------
        // Packetize data
        //----------------------------------------------------------------------
        char transmit_buffer[TLMX_MAX_BUFFER];
        bzero( transmit_buffer, TLMX_MAX_BUFFER );
        int packed_size = tlmx_trans_ptr->pack( transmit_buffer );
        sc_core::sc_assert( packed_size == unpacked_size );

        //----------------------------------------------------------------------
        // Send packet to remote
        //----------------------------------------------------------------------
        REPORT( NOTE, "Sending..." );
        int send_count = write( incoming_socket, transmit_buffer, packed_size );
        if( send_count < 0 ) {
          REPORT( ERROR, "TCPIP write/send failed" << strerror( errno ) );
        }
        sc_core::sc_assert( send_count == packed_size );

      }//endforever server_loop
    }//endforever accept_loop

    //------------------------------------------------------------------------
    // If shutdown, exit
    //------------------------------------------------------------------------
  }

  void end_of_simulation( void )
  {
    // Send shutdown to os thread
    // Close connection
  }

  // Methods
  virtual bool can_put( void ) const override
  {
    return m_ready;
  }

  virtual const sc_core::sc_event& ok_to_put( void ) const override
  {
    return m_ready_event;
  }

  virtual void put( const Async_payload& the_payload ) override
  {
    while ( not nb_put( the_payload ) ) wait( m_ready_event );
  }

  virtual bool nb_put( const Async_payload& the_payload ) override
  {
  }

private:
  // Local methods - NONE

  // Local channels
  Async_tx_channel tx;

  // Attributes
  uint32_t    m_tx_port;
  std::thread m_tx_osthread;
  std::mutex  m_tx_mutex;
  sc_core::sc_event m_ready_event;
  bool        m_ready{ false };

  static constexpr const std::string SECRET { "Mellon a minno" };// Speak "Friend and enter"
  static constexpr const char* const MSGID  { "/Doulos/Example/Tcpip_tx" };
};//end Tcpip_tx_channel

#endif/*TCPIP_TX_HPP*/

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-APACHE.txt.
//END tcpip_tx.hpp @(#)$Id$
