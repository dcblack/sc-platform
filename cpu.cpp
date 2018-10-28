////////////////////////////////////////////////////////////////////////////////
//
//   ####  #####  #    #        #     # ####   #                                 
//  #    # #    # #    #        ##   ## #   #  #                                 
//  #      #    # #    #        # # # # #    # #                                 
//  #      #####  #    #        #  #  # #    # #                                 
//  #      #      #    #        #     # #    # #                                 
//  #    # #      #    #        #     # #   #  #                                 
//   ####  #       ####  ###### #     # ####   #####                             
//
////////////////////////////////////////////////////////////////////////////////
#include "cpu.hpp"
#include "report.hpp"
#include "common.hpp"
#include "util.hpp"
#include "hexfile.hpp"
#include "memory_map.hpp"
#include "interrupt.hpp"
#include "timer_reg.hpp"
#include "timer_api.hpp"

#include <string>
#include <mutex>

namespace {
  const char* MSGID{ "/Doulos/Example/TLM-cpu" };
}
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace std;

//------------------------------------------------------------------------------
// Constructor
Cpu_module::Cpu_module( sc_module_name instance_name )
: m_mm                  { Memory_manager<>::instance()   }
, m_init_peq            { this, &Cpu_module::init_peq_cb }
, m_request_in_progress { nullptr                        }
{
  SC_HAS_PROCESS( Cpu_module );
  SC_THREAD( cpu_thread );
  SC_THREAD( irq_thread );
  init_socket.register_nb_transport_bw           ( this, &Cpu_module::nb_transport_bw );
  init_socket.register_invalidate_direct_mem_ptr ( this, &Cpu_module::invalidate_direct_mem_ptr );
  intrq_xport.bind( intrq_chan );
  m_qk.reset();
  INFO( ALWAYS, "Constructed " << name() );
}

////////////////////////////////////////////////////////////////////////////////
// Processes

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//
//   ####  #####  #    #        ####### #    # #####  #####    #    ####         
//  #    # #    # #    #           #    #    # #    # #       # #   #   #        
//  #      #    # #    #           #    #    # #    # #      #   #  #    #       
//  #      #####  #    #           #    ###### #####  ##### #     # #    #       
//  #      #      #    #           #    #    # #  #   #     ####### #    #       
//  #    # #      #    #           #    #    # #   #  #     #     # #   #        
//   ####  #       ####  ######    #    #    # #    # ##### #     # ####         
//
////////////////////////////////////////////////////////////////////////////////

#define TEST_RAM(w,addr,value) do {                             \
  write##w( RAM_BASE+addr, value );                             \
  data##w = int##w##_t(~value);                                 \
  read##w ( RAM_BASE+addr, data##w );                           \
  if( value != data##w ) REPORT( WARNING, "Data mismatch!" );   \
  MESSAGE( "wrote " << addr << ":" << HEX <<  value );        \
  MESSAGE( "read "  << addr << ":" << HEX << int(data##w) );  \
  MEND( MEDIUM );                                               \
} while(0)

#define TEST_TIMER(t) do {                                        \
  MESSAGE( "Timer " << t.timer()                                  \
    << " is " << (t.is_running()?"":" not") << " running.\n" );   \
  MESSAGE( "  Current status is " << HEX << t.status() << "\n" ); \
  MESSAGE( "  Current count  is " << DEC << t.value() << "\n" );  \
  MEND( MEDIUM );                                                 \
} while(0)

//------------------------------------------------------------------------------
void
Cpu_module::cpu_thread( void )
{

  MESSAGE( "\n" );
  RULER( 'M' );
  INFO( MEDIUM, "Testing writing/reading memory" );
  uint32_t data32;
  uint16_t data16;
  uint8_t  data8;
  TEST_RAM( 8,  0, 0xEF );
  TEST_RAM( 8,  1, 0xBE );
  TEST_RAM( 16, 2, 0xCAFE );
  data32 = 0u;
  read32( RAM_BASE + 0, data32 );
  INFO( MEDIUM, "read 0:" << HEX << data32 );
  vector<short> v1( 8 );
  v1 = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 };
  INFO( MEDIUM, "v1 = " << HEX << v1 );
  write( RAM_BASE + 0x100, v1 );
  vector<int> v2( 4 );
  read( RAM_BASE + 0x100, v2 );
  INFO( MEDIUM, "v2 = " << HEX << v2 );

  INFO( MEDIUM, "Testing hexfile functions" );

  for ( int i = 3000; i < 3050; ++i ) {
    v2.push_back( i );
  }

  hexfile::dump( 0, v2 );
  hexfile::save( "test.dat", 100, v2 );
  vector<uint8_t> v3;
  Addr_t a = hexfile::load( "test.dat", v3 );
  INFO( MEDIUM, "a=" << a << " v3.size=" << v3.size() );
  hexfile::dump( a, v3 );

  MESSAGE( "\n" );
  RULER( 'T' );
  INFO( MEDIUM, "Testing timers" );
  Timer_api t0{ *this };
  t0.setup( 10 );
  TEST_TIMER( t0 );
  t0.start();

  Timer_api t1{ *this };
  t1.setup( 30 );
  TEST_TIMER( t1 );
  t1.start();

  clk.wait_posedge( 50 );
  TEST_TIMER( t0 );
  TEST_TIMER( t1 );

  sc_stop();
}

//------------------------------------------------------------------------------
void
Cpu_module::irq_thread( void )
{
  Timer_api t0{ *this, 0 };
  for(;;) {
    intrq_chan.wait();
    INFO( MEDIUM, "Received interrupt at " << sc_time_stamp() );
    TEST_TIMER(t0);
  }//endforever
}

////////////////////////////////////////////////////////////////////////////////
// Backward interface

//------------------------------------------------------------------------------
tlm_sync_enum
Cpu_module::nb_transport_bw
( tlm_payload_t& trans
, tlm_phase& phase
, sc_time& delay
)
{
  m_init_peq.notify( trans, phase, delay );
  return TLM_ACCEPTED;
}

//------------------------------------------------------------------------------
void
Cpu_module::invalidate_direct_mem_ptr
( Addr_t start_range
, Addr_t end_range
)
{
  for ( size_t i = 0; i < dmi_table.size(); i++ ) {
    Addr_t  dmi_start_address = dmi_table[i].get_start_address();
    Addr_t  dmi_end_address   = dmi_table[i].get_end_address();

    if ( start_range <= dmi_end_address && end_range >= dmi_start_address )
      // Invalidate entire region
    {
      dmi_table.erase( dmi_table.begin() + i );
    }

  }
}

////////////////////////////////////////////////////////////////////////////////
// Overrides
void
Cpu_module::start_of_simulation ( void )
{
  m_stat["lowest address was 0x"] = MAX_ADDR;
}

//------------------------------------------------------------------------------
void
Cpu_module::end_of_simulation ( void )
{
  MESSAGE( "\n" );
  RULER  ( '#' );
  MESSAGE( "Summary for " << name() );
  for( const auto& v: m_stat ) {
    if ( v.first.find(" 0x") != string::npos ) {
      MESSAGE( "\n  " << v.first << hex << v.second );
    } else {
      MESSAGE( "\n  " << v.second << " " << v.first );
    }
  }
  MEND(ALWAYS);
}

//------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
// Helpers

//------------------------------------------------------------------------------
void
Cpu_module::init_peq_cb
( tlm_payload_t& trans
, const tlm_phase& phase
)
{
  if ( phase == END_REQ
     or ( &trans == m_request_in_progress && phase == BEGIN_RESP )
  )
  {
    // The end of the BEGIN_REQ phase
    m_request_in_progress = nullptr;
    m_end_request_event.notify();
  }
  else if ( phase == BEGIN_REQ || phase == END_RESP )
    REPORT( FATAL, "Illegal transaction phase received by " << name() );

  if ( phase == BEGIN_RESP )
  {
    INFO( DEBUG, "Got response" );
    check_transaction( trans );

    // Send final phase transition to target
    tlm_phase fw_phase = END_RESP;
    sc_time delay = sc_time( SC_ZERO_TIME );
    init_socket->nb_transport_fw( trans, fw_phase, delay );
    // Ignore return value

    // Allow the memory manager to free the transaction object
    trans.release();
  }
}

//------------------------------------------------------------------------------
// Called on receiving BEGIN_RESP or TLM_COMPLETED
void Cpu_module::check_transaction( tlm_payload_t& trans )
{
  if ( trans.is_response_error() ) {
    REPORT( ERROR, "Transaction returned with error, response status = "
                  << trans.get_response_string()
    );
  }
  m_transport_done_event.notify();
}

//------------------------------------------------------------------------------
void
Cpu_module::record_transaction
( tlm_command command
, Addr_t      address
, Depth_t     data_len
, Style       coding_style
)
{
  switch ( command ) {
    default:
      m_stat["other transactions"]++;
      m_stat["other byte manipulations"] += data_len;
      break;
    case TLM_READ_COMMAND:
      m_stat["read transactions"]++;
      m_stat["bytes read"] += data_len;
      break;
    case TLM_WRITE_COMMAND:
      m_stat["write transactions"]++;
      m_stat["bytes written"] += data_len;
      break;
  }
  switch ( coding_style ) {
    default:
      m_stat["other style transactions"]++;
      break;
    case Style::LT :
      m_stat["LT style transactions"]++;
      break;
    case Style::AT :
      m_stat["AT style transactions"]++;
      break;
  }
  if ( m_stat["highest address was 0x"] < address+data_len ) {
    m_stat["highest address was 0x"] = address+data_len;
  }
  if ( m_stat["lowest address was 0x"] > address ) {
    m_stat["lowest address was 0x"] = address;
  }
}

//------------------------------------------------------------------------------
void
Cpu_module::transport
( tlm_command command
, Addr_t      address
, uint8_t*    data_ptr
, Depth_t     data_len
, Style       coding_style
)
{
  // Prevent overlapping process calls
  std::lock_guard<sc_mutex> lock(m_transport_mutex);

  // Determine AT/LT and burst size
  if ( coding_style == Style::DEFAULT ) {
    coding_style = m_coding_style;
  }

  record_transaction( command, address, data_len, coding_style );

  sc_time delay;

  //////////////////////////////////////////////////////////////////////////////
  //
  //  #     ###   ###   ###  #### #  #     #    ####### ### #     # #### ###   
  //  #    #   # #   # #   # #    #   #   #        #     #  ##   ## #    #  #  
  //  #    #   # #   # #     #    #    # #         #     #  # # # # #    #   # 
  //  #    #   # #   #  ###  #### #     #          #     #  #  #  # #### #   # 
  //  #    #   # #   #     # #    #     #          #     #  #     # #    #   # 
  //  #    #   # #   # #   # #    #     #          #     #  #     # #    #  #  
  //  ####  ###   ###   ###  #### ####  #          #    ### #     # #### ###   
  //
  //////////////////////////////////////////////////////////////////////////////
  if ( coding_style == Style::LT ) {
    if ( coding_style != m_prev_style ) {
      INFO( DEBUG,"Loosely timed transport" );
      m_prev_style = coding_style;
    }
    bool dmi = false;
    // Check DMI table
    uint8_t* dmi_pointer;
    Addr_t   dmi_start_address;
    Addr_t   dmi_end_address;
    sc_time  dmi_latency;

    for ( size_t i = 0; i < dmi_table.size(); i++ ) {
      dmi_pointer       = dmi_table[i].get_dmi_ptr();
      dmi_start_address = dmi_table[i].get_start_address();
      dmi_end_address   = dmi_table[i].get_end_address();

      if ( address >= dmi_start_address && address <= dmi_end_address ) {
        if ( command == TLM_WRITE_COMMAND && dmi_table[i].is_write_allowed() ) {
          dmi = true;
          dmi_latency = dmi_table[i].get_write_latency();
          break;
        }
        else if ( command == TLM_READ_COMMAND && dmi_table[i].is_read_allowed() ) {
          dmi = true;
          dmi_latency = dmi_table[i].get_read_latency();
          break;
        }
        else {
          ;//TLM_IGNORE_COMMAND
        }
      }
    }
    if ( dmi ) {
      // DMI transaction
      if ( command == TLM_WRITE_COMMAND ) {
        memcpy( dmi_pointer + address - dmi_start_address, data_ptr, data_len );
      }
      else if ( command == TLM_WRITE_COMMAND ) {
        memcpy( data_ptr, dmi_pointer + address - dmi_start_address, data_len );
      }

      m_qk.inc( dmi_latency );

    }
    else {
      // Regular transaction
      //
      // Grab a new transaction from the memory manager
      tlm_payload_t& trans{ *m_mm.allocate_acquire() };
      trans.set_command        ( command  );
      trans.set_address        ( address  );
      trans.set_data_ptr       ( data_ptr );
      trans.set_data_length    ( data_len );
      trans.set_streaming_width( data_len );
      trans.set_byte_enable_ptr( nullptr  );
      trans.set_dmi_allowed    ( false    );
      trans.set_response_status( TLM_INCOMPLETE_RESPONSE );

      sc_time delay = m_qk.get_local_time();
      init_socket->b_transport( trans, delay );
      if ( trans.is_response_error() ) {
        REPORT( ERROR, "Transaction returned with error, "
                << "response status = " << trans.get_response_string()
                << "\n  command: " << to_string(command)
                << "\n  address: " << HEX << address
                << "\n  datalen: " << DEC << data_len
                << "\n  sc_time: " << sc_time_stamp()
                << "\n  delayed: " << delay
        );
      }
      trans.release();
      m_qk.set_and_sync( delay );

      // React to DMI hint
      if ( trans.is_dmi_allowed() ) {
        // Reuse transaction object to request DMI
        trans.set_address( address );
        tlm_dmi dmi_data;

        if ( init_socket->get_direct_mem_ptr( trans, dmi_data ) ) {
          dmi_table.push_back( dmi_data );
        }
      }
    }
  }
  //////////////////////////////////////////////////////////////////////////////
  //
  //     #    ####  ####  ####   ###  #     #     ####### ### #     # #### ###   
  //    # #   #   # #   # #   # #   #  #   #         #     #  ##   ## #    #  #  
  //   #   #  #   # #   # #   # #   #   # #          #     #  # # # # #    #   # 
  //  #     # ####  ####  ####  #   #    #           #     #  #  #  # #### #   # 
  //  ####### #     #     # #   #   #   # #          #     #  #     # #    #   # 
  //  #     # #     #     #  #  #   #  #   #         #     #  #     # #    #  #  
  //  #     # #     #     #   #  ###  #     #        #    ### #     # #### ###   
  //
  //////////////////////////////////////////////////////////////////////////////
  else if ( coding_style == Style::AT ) {
    if ( coding_style != m_prev_style ) {
      INFO( DEBUG, "Approximately timed transport" );
      m_prev_style = coding_style;
    }

    Cpu_module::tlm_phase_t phase;
    sc_time delay{ SC_ZERO_TIME };
    // Grab a new transaction from the memory manager
    tlm_payload_t& trans{ *m_mm.allocate_acquire() };
    trans.set_command        ( command  );
    trans.set_address        ( address  );
    trans.set_data_ptr       ( data_ptr );
    trans.set_data_length    ( data_len );
    trans.set_streaming_width( data_len );
    trans.set_byte_enable_ptr( nullptr  );
    trans.set_dmi_allowed    ( false    );
    trans.set_response_status( TLM_INCOMPLETE_RESPONSE );

    // Initiator must honor BEGIN_REQ/END_REQ exclusion rule
    if ( m_request_in_progress != nullptr ) {
          wait( m_end_request_event );
    }
    m_request_in_progress = &trans;
    phase = BEGIN_REQ;

    // Non-blocking transport call on the forward path
    INFO( DEBUG, "Beginning nb_transport_fw at " << sc_time_stamp() );
    tlm_sync_enum status;
    status = init_socket->nb_transport_fw( trans, phase, delay );

    // Check value returned from nb_transport_fw
    if ( status == TLM_UPDATED ) {
      // The timing annotation must be honored
      m_init_peq.notify( trans, phase, delay );
      wait( m_transport_done_event );
    }
    else if ( status == TLM_COMPLETED ) {
      // Must not start until correct time
      if ( delay != SC_ZERO_TIME ) {
        wait( delay );
      }
      // The completion of the transaction necessarily ends the BEGIN_REQ phase
      m_request_in_progress = nullptr;

      // The target has terminated the transaction
      INFO( DEBUG, "Early completion" );
      check_transaction( trans );

      // Allow the memory manager to free the transaction object
      trans.release();
    }
    else {
     
      wait( m_transport_done_event );
    }
  } else {
    REPORT( FATAL, "Uknown coding style" );
  }
}

//------------------------------------------------------------------------------
void Cpu_module::put( Addr_t address, std::vector<uint8_t>& vec )
{
  sc_assert( vec.size() > 0 );
  transport_dbg(TLM_WRITE_COMMAND, address, vec);
}

//------------------------------------------------------------------------------
void Cpu_module::get( Addr_t address, Depth_t depth, std::vector<uint8_t>& vec )
{
  sc_assert( depth > 0 );
  vec.resize( depth );
  transport_dbg(TLM_READ_COMMAND, address, vec);
}

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
