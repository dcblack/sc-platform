#include <systemc>
#include <sstream>
#include <iomanip>
using namespace sc_core;
using namespace std;

stringstream mout;

#define INFO(stream) do {\
  mout << stream << ends;
  SC_REPORT_INFO( "/Doulos/Example/Sigvec", mout.str().c_str() );
  mout.str("");
} while(0)


////////////////////////////////////////////////////////////////////////////////
SC_MODULE(Splitter_module) {
  // Ports
  sc_in<uint32_t> in32_port;
  sc_vector<sc_export<sc_signal_in_if<bool>>> out_xport;

  // Channels
  sc_vector<sc_signal<bool>> out_sig;

  // Register processes
  SC_CTOR(Splitter_module) {
    SC_METHOD(update_method);
      sensitive << in32_port;
    out_xport.bind( out_sig );
  }

  // Process
  void update_method( void ) {
    uint32 in32 = in32_port->read();
    for( auto xp : out_xport ) {
      xp->write( in32 & 1 );
      in32 >>= 1;
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
SC_MODULE(Src_module) {
  sc_out<uint32_t> out_port;
  SC_CTOR(Src_module) {
    SC_METHOD(src_method);
      sensitive << in_port;
      dont_initialize();
  }
  void src_method( void ) {
    vector<uint32_t> data_vec =
    { 0x0000'0000
    , 0x4000'0000
    , 0x0008'0000
    , 0xDEAD'BEEF
    , 0x0000'0000
    };
    wait( 10, SC_NS );
    for( auto item : data_vec ) {
      out_port->write( item );
      wait( 10, SC_NS );
    }
    sc_stop();
  }
};

////////////////////////////////////////////////////////////////////////////////
SC_MODULE(Sink_module) {
  sc_in<bool> in_port;
  SC_CTOR(Sink_module) {
    SC_METHOD(sink_method);
      sensitive << in_port.pos();
      dont_initialize();
  }
  void sink_method( void ) {
    INFO( __func__ << " received " << in_port->read() << " at " << sc_time_stamp() );
  }
};

////////////////////////////////////////////////////////////////////////////////
SC_MODULE(Top_module) {

  // Channels
  sc_signal<uint32_t> sig32;
  sc_vector< sc_signal<bool> > bit{ 32 };

  // Sub-modules
  Src_module       src{ "src" };
  Splitter_module  split{ "split" };
  sc_vector<Sink_module> snk{ 32 };

  // Connectivity
  SC_CTOR(Top_module) {
    src.out_port.bind( sig32 );
    split.in32_port
    split.out_xport.bind( bit );
  }

};

////////////////////////////////////////////////////////////////////////////////

int sc_main( int argc, const char* argv[] )
{
  unique_ptr<Top_module> top{ new Top_module{"top"};
  sc_start();
  return 0;
}
