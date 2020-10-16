#pragma once

#include <systemcc>
#include <string>

struct Commandline
{
  // Return index if a command-line argument beginning with opt exists; otherwise, zero
  inline static size_t has_opt( std::string opt )
  {
    for( int i = 1; i < sc_core::sc_argc(); ++i ) {
      std::string arg{ sc_core::sc_argv()[ i ] };
      if( arg.find( opt ) == 0 ) return i;
    }
    return 0;
  }
private:
  [[maybe_unused]]inline constexpr static char const * const
  MSGID{ "/Doulos/Example/Commandline" };
};
