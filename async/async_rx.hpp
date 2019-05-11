/**
 * @file async_rx.hpp
 *
 * @author David C Black
 * @date   Thu Apr 11 2019
 *
 * {:COMMENT:}
 */

#ifndef ASYNC_RX_HPP
#define ASYNC_RX_HPP

#include "async/async_if.hpp"
#include "async/async_payload.hpp"
#include <systemc>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

/eda/hldw/default/bin/new doxy-class Async_rx_channel
/**
 * @class Async_rx_channel
 *
 * @brief {:BRIEF:}
 *
 * {:DETAILS:}
 *
 */
template<typename T>
struct Async_rx_channel
: sc_core::sc_prim_channel
, Async_if<T>
{
  Async_rx_channel( const std::string& instance_name ); ///< Constructor
  ~Async_rx_channel( void ); ///< Destructor

  // Safely add entry to queue and return success status (in case queue full)
  bool notify( const Async_payload<T>& the_payload ) override;

  // Performs the transfer during SystemC update phase
  // *** Do NOT call this directly ***
  void update( void );
  `
  // Determines if there is anything waiting to be read
  bool is_empty( void ) const override;

  // Block if no events are available
  void wait_unless_available( void ) const override;

  // Returns # of events available
  size_t available( void ) override;

  // Only valid for SystemC side.
  const sc_core::sc_event& default_event( void ) override;

  // Copy out data -- returns false if queue empty. Sets data_len to 0 if no data. Removes entry from queue if successful.
  bool nb_read( Async_payload<T>& the_payload ) override;

  // Blocking call -- waits for data. Sets data_len to 0 if no data. Removes entry from queue.
  void read( Async_payload<T>& the_payload ) override;
 
  // Get point to data -- returns false if queue empty. Does NOT complete transaction.
  bool nb_peek( Async_payload<T>*& payload_ptr ) override;

  // Blocking call to get pointer to data -- waits for data. Does NOT complete transaction.
  void peek( Async_payload<T>*& payload_ptr ) override;

  // Toss the payload at the head of the queue. Completes the transaction.
  // Requires that peek has been called
  // DANGEROUS -- better to use nb_read
  void drop( void ) override;


private:
  // Attributes
  std::queue< Async_payload<T> > m_queue;
  std::mutex                     m_queue_mutex;
  std::mutex                     m_condition_mutex;
  std::condition_variable        m_condition_variable; // indicates a read event
  bool                           m_ready{ false };
  sc_core::sc_event              m_event; // indicates a write event
  bool                           m_peeked{ false };

  static constexpr const char* const MSGID
  { "/Doulos/Async_rx_channel" };
};//end Async_rx_channel

// Implementation
//------------------------------------------------------------------------------
// Constructor
template<typename T>
Async_rx_channel<T>::Async_rx_channel( const std::string& instance_name )
: sc_core::sc_prim_channel( instance_name.c_str() )
{
  REPORT_INFO( "Constructed incoming " << name() );
}

// Destructor
template<typename T>
Async_rx_channel<T>::~Async_rx_channel( void )
{
  REPORT_INFO( "Destroyed incoming " << name() );
}

// Safely add entry to queue and return success status (in case queue full)
template<typename T>
bool Async_rx_channel<T>::notify( const Async_payload<T>& the_payload ) override
{
  {
    std::lock_guard<std::mutex> the_lock( m_queue_mutex );
    m_queue.push_back( the_payload );
    m_ready = true;
    async_request_update();
  }
}

template<typename T>
void Async_rx_channel<T>::update( void )
{
  m_event.notify();
}

template<typename T>
bool Async_rx_channel<T>::is_empty( void ) const override
{
  std::lock_guard<std::mutex> the_lock( m_queue_mutex );
  return m_queue.empty();
}

// Blocks if no events are available
template<typename T>
void Async_rx_channel<T>::wait_unless_available( void ) const override
{
  if ( is_empty() ) {
    std::lock_guard<std::mutex> the_lock( m_condition_mutex );
    while( not m_ready ) wait( m_event );
  }
}

// Returns # of events available
template<typename T>
size_t Async_rx_channel<T>::available( void ) override
{
  size_t size;
  {
    std::lock_guard<std::mutex> the_lock( m_queue_mutex );
    size = m_queue.size();
  }
  return size;
}

// Only valid for SystemC side.
template<typename T>
const sc_core::sc_event& Async_rx_channel<T>::default_event( void ) override
{
  return m_event;
}

// Copy out data -- returns false if queue empty. Sets data_len to 0 if no data. Removes entry from queue if successful.
template<typename T>
bool Async_rx_channel<T>::nb_read( Async_payload<T>& the_payload ) override
{
  std::lock_guard<std::mutex> the_lock( m_queue_mutex );
  if ( m_queue.empty() ) return false;
  the_payload = m_queue.front();
  m_queue.pop_front();
  m_peeked = false;
  return true;
}

// Blocking call -- waits for data. Sets data_len to 0 if no data. Removes entry from queue.
template<typename T>
void Async_rx_channel<T>::read( Async_payload<T>& the_payload ) override
{
  wait_unless_available();
  (void) nb_read( the_payload );
}

// Get point to data -- returns false if queue empty. Does NOT complete transaction.
template<typename T>
bool Async_rx_channel<T>::nb_peek( Async_payload<T>*& payload_ptr ) override
{
  std::lock_guard<std::mutex> the_lock( m_queue_mutex );
  if ( m_queue.empty() ) return false;
  payload_ptr = &m_queue.front();
  m_peeked = true;
  return true;
}

// Blocking call to get pointer to data -- waits for data. Does NOT complete transaction.
template<typename T>
void Async_rx_channel<T>::peek( Async_payload<T>*& payload_ptr ) override
{
  wait_unless_available();
  (void) nb_peek( payload_ptr );
}

// Toss the payload at the head of the queue. Completes the transaction.
// Requires that peek has been called
// DANGEROUS -- better to use nb_read
template<typename T>
void Async_rx_channel<T>::drop( void ) override
{
  if ( not m_peeked ) return;
  std::lock_guard<std::mutex> the_lock( m_queue_mutex );
  if ( not m_queue.empty() ) {
    m_queue.pop_front();
    m_event.notify();
  }
}

#endif/*ASYNC_RX_HPP*/

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-apache.txt.
//END async_tx.hpp @(#)$Id$
