#ifndef INTERRUPT_IF_HPP
#define INTERRUPT_IF_HPP

#include <systemc>

struct Interrupt_send_if
: virtual sc_core::sc_interface
{
  // Send an event using the current module instance as source
  virtual void notify( void ) = 0; //< immediate
  virtual void notify( const sc_core::sc_time& delay ) = 0;
  // Send an event from the specified source
  virtual void notify( const char* source ) = 0; //< immediate
  virtual void notify( const char* source, const sc_core::sc_time& delay ) = 0;
};

struct Interrupt_recv_if
: virtual sc_core::sc_interface
{
  // Wait for an event. *** Does NOT wait if already available ***
  virtual void   wait        ( void ) = 0;
  // Take an event off the queue and return the name of the source
  virtual char*  get_next    ( void ) = 0;
  // Combination of wait & get_next
  virtual void   wait_n_get  ( char** source ) = 0;
  // Empty the entire queue (e.g. for reset
  virtual void   clearall    ( void ) = 0;
  // Number of calls to notify
  virtual size_t sent        ( void ) const = 0;
  // Number of calls to get
  virtual size_t taken       ( void ) const = 0;
  // Interrupts are pending immediate service at the current simulation time
  virtual bool   has_pending ( void ) const = 0;
};

struct Interrupt_debug_if
: virtual sc_core::sc_interface
{
  virtual const sc_core::sc_event& event( void ) const = 0;
  // Interrupts needing immediate service at the current simulation time
  virtual size_t pending     ( void ) const = 0;
  // Total scheduled interrupts
  virtual size_t outstanding ( void ) const = 0;
};

#endif /*INTERRUPT_IF_HPP*/
