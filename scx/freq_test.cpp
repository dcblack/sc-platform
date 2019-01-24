#include "scx/sc_freq.h"
#include <systemc>
#include <utility>
#include <iostream>
using namespace sc_core;

namespace {
  const char* const MSGID{ "/Doulos Inc/freq_test/main" };
}

#define SHOW(f) std::cout << #f << "=" << f << std::endl

int sc_main( int argc, char* argv[] )
{

  sc_freq f1{ 1.0, SC_MHZ };
  sc_freq f2{ 2.0_KHz };
  sc_freq f6{ 6_KHz };
  sc_freq freq{ f2 };
  {
    sc_freq f0;
    SHOW(f0);
    f1 += std::move(f0);
  }

  SHOW(f1);
  SHOW(f2);
  SHOW(f6);

  SHOW(freq);

  freq = 2*f2 + f6/2.0;

  SHOW(freq);

  sc_time t1 = 1.0/f1;

  SHOW(f1);
  SHOW(f1.units());
  SHOW(f1.value(f1.units()));
  SHOW(f1.value(SC_HZ));
  SHOW(t1);

  t1 = 200_MHz;
  SHOW(t1);

  t1 = -5_KHz;
  SHOW(t1);

  return sc_report_handler::get_count( SC_ERROR )
         + sc_report_handler::get_count( SC_FATAL );
}
