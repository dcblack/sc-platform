#include "report.hpp"

struct Top_module: sc_core::sc_module
{
  Top_module( sc_core::sc_module_name instance )
  : sc_module( instance )
  {
  }
};

using namespace sc_core;

int sc_main( int argc, char* argv[] )
{
  return 0;
}
