#ifndef INTERRUPT_HPP
#define INTERRUPT_HPP

#include "interrupt/interrupt_if.hpp"
#include <map>

struct Interrupt_channel
: private Interrupt_send_if
, private Interrupt_recv_if
, private Interrupt_debug_if
, sc_core::sc_object
{
  // Exported interfaces
  sc_core::sc_export< Interrupt_send_if>  send_if  { "send_if"  };
  sc_core::sc_export< Interrupt_recv_if>  recv_if  { "recv_if"  };
  sc_core::sc_export< Interrupt_debug_if> debug_if { "debug_if" };

  //----------------------------------------------------------------------------
  // Constructor
  Interrupt_channel( const std::string& instance_name )
  : sc_core::sc_object( instance_name.c_str() )
  {
    send_if.bind ( *this );
    recv_if.bind ( *this );
    debug_if.bind( *this );
  }

  //----------------------------------------------------------------------------
  ~Interrupt_channel( void ) = default;

private:
  //----------------------------------------------------------------------------
  void notify( void )
  {
    notify( originator() );
  }

  //----------------------------------------------------------------------------
  void notify( const sc_core::sc_time& delay )
  {
    notify( originator(), delay );
  }

  //----------------------------------------------------------------------------
  void notify( const char* source )
  {
    m_scheduled_events.insert(std::make_pair(sc_core::sc_time_stamp(), const_cast<char*>(source)));
    m_event.notify();
    ++m_sent;
  }

  //----------------------------------------------------------------------------
  void notify( const char* source, const sc_core::sc_time& delay )
  {
    m_scheduled_events.insert(std::make_pair(delay + sc_core::sc_time_stamp(), const_cast<char*>(source)));
    ++m_sent;
  }

  //----------------------------------------------------------------------------
  void wait_n_get( char** source = nullptr )
  {
    this->wait();
    auto result = get_next();
    if( source != nullptr ) *source = result;
  }

  //----------------------------------------------------------------------------
  void wait( void )
  {
    if( pending() == 0 ) sc_core::wait( m_event );
  }

  //----------------------------------------------------------------------------
  char* get_next( void )
  {
    if (m_scheduled_events.empty()) {
      return nullptr;
    }

    auto now = sc_core::sc_time_stamp();
    auto oldest_event = m_scheduled_events.begin();
    if (oldest_event->first <= now) {
      auto result = oldest_event->second;
      m_scheduled_events.erase(oldest_event);
      ++m_taken;
      return result;
    }

    m_event.notify(m_scheduled_events.begin()->first - now);

    return nullptr;
  }

  //----------------------------------------------------------------------------
  void clearall ( void )
  {
    m_scheduled_events.clear();
    m_event.cancel();
  }

  //----------------------------------------------------------------------------
  bool has_pending ( void ) const
  {
    if (m_scheduled_events.empty()) return false;
    return m_scheduled_events.begin()->first <= sc_core::sc_time_stamp();
  }

  //----------------------------------------------------------------------------
  size_t pending ( void ) const
  {
    size_t result = 0;
    if (m_scheduled_events.empty()) {
      return result;
    }
    auto now = sc_core::sc_time_stamp();
    for( const auto& event : m_scheduled_events ) {
      if ( event.first > now ) break;
      ++result;
    }
    return result;
  }

  //----------------------------------------------------------------------------
  size_t outstanding ( void ) const
  {
    return m_scheduled_events.size();
  }

  //----------------------------------------------------------------------------
  size_t sent ( void ) const { return m_sent; }

  //----------------------------------------------------------------------------
  size_t taken ( void ) const { return m_taken; }

  //----------------------------------------------------------------------------
  // Following provided for debug only
  const sc_core::sc_event& event( void ) const { return m_event; }

private:

  //----------------------------------------------------------------------------
  // Helpers
  const char* originator( void ) const
  {
    auto process_h = sc_core::sc_get_current_process_handle();
    auto object_ptr = process_h.get_parent_object();
    do {
      auto module_ptr = dynamic_cast< sc_core::sc_module* >( object_ptr );
      if( module_ptr != nullptr ) {
        return module_ptr->name();
      }
      object_ptr = object_ptr->get_parent_object();
    } while( object_ptr != nullptr );
    return "<UNKNOWN>";
  }

  //----------------------------------------------------------------------------
  // Attributes
  using sched_map_t = std::multimap<const sc_core::sc_time, char*>;
  size_t            m_sent  { 0u };
  size_t            m_taken { 0u };
  sched_map_t       m_scheduled_events;
  sc_core::sc_event m_event;
};

#endif /*INTERRUPT_HPP*/
