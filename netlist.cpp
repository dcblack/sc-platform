#include "netlist.hpp"
#include "report.hpp"
#include <vector>
#include <string>
using namespace sc_core;
namespace {
  const char* const MSGID{ "/Doulos/Example/Netlist" };
}

namespace {
  // Traverse the entire object subhierarchy 
  // below a given object 
  void scan_hierarchy(sc_object* obj,std::string indent)
  { 
    std::vector<sc_object*> children = obj->get_child_objects(); 
    for( const auto& child : children ) {
      if ( child != nullptr ) {
        MESSAGE( indent << "+- " << child->basename() << ' ' << child->kind() << "\n" );
        scan_hierarchy(child, indent + std::string("| ")); 
      }//endif
    }//endfor
  }//end scan_hierarchy()
}

Netlist::Netlist( void )
{
  // Traverse the object hierarchy below each top-level object
  std::vector<sc_object*> top_vec = sc_get_top_level_objects();
  MESSAGE( "Netlist {\n" );

  for ( const auto& top : top_vec ) {
    if ( top != nullptr ) {
      MESSAGE( "  " <<  top->name() << ' ' << top->kind() << "\n" );
      scan_hierarchy( top, "    " );
    }//endif
  }//endfor

  INFO( MEDIUM, "} end Netlist\n" );
}

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END main.cpp $Id$
