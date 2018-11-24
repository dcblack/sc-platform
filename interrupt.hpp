#ifndef INTERRUPT_HPP
#define INTERRUPT_HPP

#include "interrupt_if.hpp"

struct Interrupt
: Interrupt_send_if
, Interrupt_recv_if
, Interrupt_debug_if
, sc_core::sc_object
{
  Interrupt( const std::string& instance_name )
  : sc_core::sc_object( instance_name.c_str() )
  {
  }
  ~Interrupt( void ) = default;
  void notify( void )
  {
    m_event.notify();
    ++m_pending;
    ++m_sent;
  }
  void notify( const sc_core::sc_time& delay )
  {
    m_event.notify(delay);
    ++m_pending;
    ++m_sent;
  }
  void wait( void )
  {
    if( m_pending == 0 ) sc_core::wait( m_event );
    --m_pending;
    ++m_active;
    ++m_rcvd;
  }
  void   clear   ( void ) { --m_active; }
  bool   active  ( void ) const { return ( m_active != 0 ); }
  bool   pending ( void ) const { return ( m_pending != 0 ); }
  size_t sent    ( void ) const { return m_sent; }
  size_t rcvd    ( void ) const { return m_rcvd; }
  // Following provided for debug only
  const sc_core::sc_event& event( void ) const { return m_event; }
private:
  sc_core::sc_event m_event;
  size_t            m_pending { 0u };
  size_t            m_active  { 0u };
  size_t            m_sent    { 0u };
  size_t            m_rcvd    { 0u };
};

#endif /*INTERRUPT_HPP*/
