#ifndef INTERRUPT_IF_HPP
#define INTERRUPT_IF_HPP

struct Interrupt_send_if
: virtual sc_core::sc_interface
{
  virtual void notify( void ) = 0;
  virtual void notify( const sc_core::sc_time& delay ) = 0;
};

struct Interrupt_recv_if
: virtual sc_core::sc_interface
{
  virtual void wait( void ) = 0;
  virtual size_t sent( void ) const = 0;
  virtual size_t rcvd( void ) const = 0;
};

struct Interrupt_debug_if
: virtual sc_core::sc_interface
{
  virtual const sc_core::sc_event& event( void ) const = 0;
};

#endif /*INTERRUPT_IF_HPP*/
