#include "async_payload.hpp"
#include "yaml-cpp/yaml.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;

#ifdef ASYNC_PAYLOAD_EXAMPLE

#include "async_payload.hpp"
#include <array>

int main( int argc, const char* argv[] )
{
  int status{ 0 };

  array<uint8_t,256> mydata;
  Async_payload empty;
  Async_payload basic{ Async_cmnd::stream, mydata.data(), sizeof(mydata)/sizeof(mydata[0]) };

  return status;
}

#endif

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-Apache.txt.
//END async_payload.cpp
