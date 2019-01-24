#include "uart/parity.hpp"
using namespace std;

namespace {
struct Parity
  {
    Parity( void )
    {
      for( auto v=0u; v!=256u; ++v )
      {
        odd_table[ v ] = odd( v );
      }
    }
    inline bool odd( uint8_t v )
    {
      static bool odd4[16] 
      { false, true,  true,  false, true,  false, false, true
      , true,  false, false, true,  false, true,  true,  false
      };
      return odd4[ v>> 4 ] ^ odd4[ v&0xf ];
    }
    bool odd_table[256];
  } parity;
};

bool odd( uint8_t octet )
{
  return parity.odd_table[ octet ];
}

bool odd( uint8_t const * const data, std::size_t depth )
{
  bool parity = false;
  for( int i=0; i!=depth; ++i ) {
    parity ^= odd( data[i] );
  }
  return parity;
}

#ifdef PARITY_TEST
#include <iostream>
#include <iomanip>
#include <string>
int main() {
  for( int r = 0; r<4; ++r )
  {
    for( int c = 0; c<64; ++c )
    {
      int v = (r<<6)+c;
      cout << ( odd( v ) ? "o" : "E" );
    }
    cout << endl;
  }
  string s = "Hello world!";
  for( char c{'\a'}; c!='\0'; c<<=1  ) {
    cout << "\"" << s << "\" is " << (odd(reinterpret_cast<uint8_t const * const>(s.data()),s.length())?"odd":"even") << endl;
    s[0] ^= c;
  }
  return 0;
}
#endif

//------------------------------------------------------------------------------
// Copyright {:YEAR:} by {:COMPANY:}. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END {:FILE:} {:Id:}
