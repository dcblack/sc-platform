#include "async/async_payload.hpp"

// Nothing here except syntax check and self-test/example

#ifdef ASYNC_PAYLOAD_EXAMPLE

using namespace std;
using namespace sc_core;
#include "async/async_payload.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#define SHOWVEC(VEC) do {        \
  std::cout << #VEC << std::hex << "\n";   \
  for( auto& val : VEC ) {       \
    std::cout << "- " << val << "\n"; \
  }                              \
  std::cout << std::dec << std::flush;           \
} while(0)

#define SHOW(V) std::cout << #V << " = " << V << std::endl

int sc_main( int argc, char* argv[] )
{
  int status{ 0 };

  using mydata_t = vector<uint32_t>;
  Async_payload<int16_t> empty;
  SHOW(empty);
  Async_payload<double> voltage;
  voltage.set_time( sc_time(3.1,SC_NS).value() );
  voltage.set_kind( Async_kind::packet );
  voltage.set_orig( 0xD0410500ull );
  SHOW(voltage);
  mydata_t mydata(4);
  for( auto& v:mydata ) v=0x12'34'56'78;
  SHOWVEC(mydata);
  Async_payload<mydata_t> mybasic{ Async_kind::stream, mydata };
  SHOW(mybasic);
  string buffer = mybasic.pack();
  SHOW(buffer);
  Async_payload<mydata_t> second;
  second.unpack( buffer );
  SHOW(second);
  SHOWVEC(second.get_data());
  Async_payload<mydata_t> third;
  third = second;
  SHOW(third);
  Async_payload<mydata_t> fourth(third);
  SHOW(fourth);

  return status;
}

#endif

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-Apache.txt.
//END async_payload.cpp
