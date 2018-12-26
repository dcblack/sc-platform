#include "report.hpp"
#include "summary.hpp"
#include "common.hpp"
#include "yaml-cpp/yaml.h"
#include <string>

using namespace std;
using namespace sc_core;
namespace {
  const char * const MSGID{ "/Doulos/Example/yaml" };
}

string nodeType( const YAML::Node& node )
{
  string result;
  switch (node.Type()) {
    case YAML::NodeType::Null:      result = "YAML::NodeType::Null";      break;
    case YAML::NodeType::Scalar:    result = "YAML::NodeType::Scalar";    break;
    case YAML::NodeType::Sequence:  result = "YAML::NodeType::Sequence";  break;
    case YAML::NodeType::Map:       result = "YAML::NodeType::Map";       break;
    case YAML::NodeType::Undefined: result = "YAML::NodeType::Undefined"; break;
  }
  return result;
}

#define SHOW(w,n,t) do {\
    if( n ) {\
      MESSAGE( "\n  " << w << " = " << n.as<t>() );\
    }\
}while(0)

#define SHOW2(w,t) do {\
    if( pt.second[w] ) {\
      MESSAGE( " " << w << "=" << pt.second[w].as<t>() );\
    }\
}while(0)

int sc_main( int argc, char* argv[] )
{
  sc_report_handler::set_actions( SC_ERROR, SC_DISPLAY | SC_LOG );
  sc_report_handler::set_actions( SC_FATAL, SC_DISPLAY | SC_LOG );

  // Defaults
  string busname{ "top.nth" };
  string filename{ "memory_map.yaml" };

  // Scan command-line
  for( int i=1; i<sc_argc(); ++i ) {
    string arg( sc_argv()[i] );
    if( arg.find(".yaml") == arg.size() - 5 ) {
      filename = arg;
    } else if( arg.find("-") != 0 ) {
      busname = arg;
    }
  }

  INFO( ALWAYS, "Searching for busname=" << busname );

  YAML::Node root;
  try {
    root = YAML::LoadFile( filename );
  } catch (const YAML::Exception& e) {
    REPORT( FATAL, e.what() );
  }

  for( const auto& p : root ) {
    if( p.first.as<string>() != busname ) continue;
    MESSAGE( p.first.as<string>());
    SHOW( "addr", p.second["addr"], string );
    SHOW( "size", p.second["size"], Depth_t );
    for( const auto& pt : p.second["targ"] ) {
      MESSAGE( pt.first.as<string>() << " => " );
      if( pt.second["addr"] )
        MESSAGE( " " << "addr" << "=" << pt.second["addr"].as<Addr_t>() );
      if( pt.second["size"] )
        MESSAGE( " " << "size" << "=" << pt.second["size"].as<string>() );
      if( pt.second["kind"] )
        MESSAGE( " " << "kind" << "=" << pt.second["kind"].as<string>() );
      if( pt.second["irq"] )
        MESSAGE( " " << "irq" << "=" << pt.second["irq"].as<int>() );
      MESSAGE( "\n" );
    }
    MEND( MEDIUM );
  }
  return Summary::report();
}
