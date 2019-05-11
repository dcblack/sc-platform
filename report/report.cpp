#include <sstream>
#include <string>
#include <systemc>
#include <tlm>
using namespace sc_core;
using namespace std;
using namespace tlm;

std::ostringstream mout;

/**
 * @func convert tlm_command enum to string
 */
string to_string( tlm_command command )
{
  switch( command )
  {
    case  TLM_WRITE_COMMAND: return  "TLM_WRITE_COMMAND";
    case   TLM_READ_COMMAND: return   "TLM_READ_COMMAND";
    case TLM_IGNORE_COMMAND: return "TLM_IGNORE_COMMAND";
    default:                 return   "!UKNOWN_COMMAND!";
  }
}

/**
 * @func output an array of bytes
 */
string to_string( uint8_t const * const data, uint32_t len )
{
  static char const * const hexdigit = "0123456789ABCDEF";
  string result( len+2, '0' );
  result[1] = 'x';
  for( int i=2; len > 0; ) {
    --len;
    uint8_t d = data[len];
    result[i++] = hexdigit[ d >>  4 ];
    result[i++] = hexdigit[ d & 0xF ];
  }
  return result;
}

/**
 * @func Convert verbosity to string
 */
std::string verbosity2str(const int & level)
{
  std::ostringstream os;
  switch( level ) {
    case SC_NONE   : os << "NONE";   break;
    case SC_LOW    : os << "LOW";    break;
    case SC_MEDIUM : os << "MEDIUM"; break;
    case SC_HIGH   : os << "HIGH";   break;
    case SC_FULL   : os << "FULL";   break;
    case SC_DEBUG  : os << "DEBUG";  break;
    default:
      if(      level < SC_LOW    ) { os << std::to_string( int( level - SC_NONE   ) ) << "% LOW"    ; }
      else if( level < SC_MEDIUM ) { os << std::to_string( int( level - SC_LOW    ) ) << "% MEDIUM" ; }
      else if( level < SC_HIGH   ) { os << std::to_string( int( level - SC_MEDIUM ) ) << "% HIGH"   ; }
      else if( level < SC_FULL   ) { os << std::to_string( int( level - SC_HIGH   ) ) << "% FULL"   ; }
      else if( level < SC_DEBUG  ) { os << std::to_string( int( level - SC_FULL   ) ) << "% DEBUG"  ; }
      else                         { os << "DEBUG + " << std::to_string( int( level - SC_DEBUG ) )  ; }
  }//endswitch
  return os.str();
}

