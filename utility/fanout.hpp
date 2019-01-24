#ifndef FANOUT_HPP
#define FANOUT_HPP
////////////////////////////////////////////////////////////////////////////////
//
//  #####    #    #     #  ####  #    # #######                                   
//  #       # #   ##    # #    # #    #    #                                      
//  #      #   #  # #   # #    # #    #    #                                      
//  ##### #     # #  #  # #    # #    #    #                                      
//  #     ####### #   # # #    # #    #    #                                      
//  #     #     # #    ## #    # #    #    #                                      
//  #     #     # #     #  ####   ####     #                                      
//
////////////////////////////////////////////////////////////////////////////////

// For more information, see `ABOUT_FANOUT.md`

#include <systemc>

template<typename T1, typename T2>
struct Fanout
: sc_core::sc_channel
, private sc_signal_inout_if<T1>
{
  using ZERO_OR_MORE = sc_core::SC_ZERO_OR_MORE_BOUND;

  // Ports
  sc_core::sc_export< sc_signal_inout_if<T1> > xport;
  sc_core::sc_vector< sc_port< sc_signal_inout_if<T2, 1, ZERO_OR_MORE> > port;

  Fanout( sc_core::sc_module_name instance, size_t width )
  {
    xport.bind( *this );
    SC_HAS_PROCESS( Fanout );
    SC_THREAD( fanout_thread );
  }

private:

  void fanout_thread( void )
  {
    sc_core::sc_event_or_list m_or;
    for( const auto& p : port ) {
      m_or |= p->value_changed_event();
    }
    for(;;) {
      wait( m_or );
      m_event.notify();
      m_delta = sc_core::sc_delta_count();
      update();
    }
  }

  void write( const T1& value ) override
  {
    for( int i=0; i<port.size(); ++i ) {
      if( port[i].size == 0 ) continue;
      port[i]->write( value[i] );
    }
  }

  const T1& read( void ) const override
  {
    update();
    return m_read;
  }

  void update( void )
  {
    for( int i=0; i<port.size(); ++i ) {
      if( port[i].size == 0 ) continue;
      m_read[i] = port[i]->read();
    }
  }

  const T& get_data_ref() const
  {
    return m_read;
  }

  const sc_core::sc_event& value_changed_event( void ) override
  {
  }

  bool event( void ) const
  {
    bool result = false;
    for( const auto& p : port ) {
      if( p.size() == 1 ) result |= p->event();
    }
  }

  //----------------------------------------------------------------------------
  // Attributes
  T1                m_read;
  sc_core::sc_event m_event;
  
};
#endif /*FANOUT_HPP*/
