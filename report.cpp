#include <sstream>
#include <string>
#include <systemc>
#include <tlm>
using namespace sc_core;
using namespace std;
using namespace tlm;

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

