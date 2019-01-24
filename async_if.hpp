#ifndef ASYNC_IF_HPP
#define ASYNC_IF_HPP

#include <systemc>
#include "async_payload.hpp"

template<typename T>
struct Async_tx_if
: virtual sc_core::sc_interface
{
  // Safely add entry to queue and return success status (in case queue full)
  virtual bool notify( const Async_payload<T>& the_payload ) = 0;
};

struct Async_rx_base_if
: virtual sc_core::sc_interface
{
  virtual void wait_unless_available( void ) = 0; // Blocks if no events are available
  virtual bool is_empty( void ) const = 0; //< Returns true if no events waiting
  virtual size_t available( void ) const = 0; //< Returns # of events available
  virtual const sc_core::sc_event& default_event( void ) const = 0; // Only valid for SystemC side.
};

template<typename T>
struct Async_rx_if
: virtual sc_core::sc_interface
, Async_rx_base_if
{
  // Copy out data -- returns false if queue empty. Sets data_len to 0 if no data. Removes entry from queue if successful.
  virtual bool nb_read( Async_payload<T>& the_payload ) = 0;
  // Blocking call -- waits for data. Sets data_len to 0 if no data. Removes entry from queue.
  virtual void read( Async_payload<T>& the_payload ) = 0;
  // Get point to data -- returns false if queue empty. Does NOT complete transaction.
  virtual bool nb_peek( Async_payload<T>*& payload_ptr ) const = 0;
  // Blocking call to get pointer to data -- waits for data. Does NOT complete transaction.
  virtual void peek( Async_payload<T>*& payload_ptr ) const = 0;
  // Toss the payload at the head of the queue. Completes the transaction.
  virtual void drop( void ) = 0;
};

template<typename T>
struct Async_if: Async_rx_if<T>, Async_tx_if<T> { }

#endif/*ASYNC_IF_HPP

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END async_if.hpp @(#)$Id$
