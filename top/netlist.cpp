#include "top/netlist.hpp"
#include <tlm>
#include "report/report.hpp"
#include "top/options.hpp"
#include <vector>
#include <string>
#include <cctype>
using namespace sc_core;
using namespace tlm;
using std::string;
namespace {
  const char* const MSGID{ "/Doulos/Example/Netlist" };
}

namespace {
  // Traverse the entire object subhierarchy 
  // below a given object 
  void scan_hierarchy(sc_object* obj, string indent)
  { 
    std::vector<sc_object*> children = obj->get_child_objects(); 
    for( const auto& child : children ) {
      if ( child != nullptr ) {
        string inst{ child->basename() };
        string kind{ child->kind() };
        // Is this a port? If so, let's find out what it is connected to...
        string channel_name{ "" };
        const sc_interface* interface_ptr = nullptr;
        // sc_export?
        auto export_ptr = dynamic_cast< const sc_export_base* >( child );
        if( export_ptr != nullptr ) {
          interface_ptr = export_ptr->get_interface();
        }
        // sc_port?
        auto port_ptr = dynamic_cast< const sc_port_base* >( child );
        if( port_ptr != nullptr ) {
          interface_ptr = port_ptr->get_interface();
        }
        if( interface_ptr != nullptr ) {
          INFO( DEBUG, "Inteface points to " << HEX << interface_ptr ); 
          auto object_ptr = dynamic_cast< const sc_object* >( interface_ptr );
          if( object_ptr != nullptr ) {
            channel_name = object_ptr->name();
          }
        }
        //{:TODO:} Filter types (unless verbose)
        bool ok = true;
        if( not Options::has_flag("-v") ) {
          size_t l = inst.length();
          ok &= !(l>2 and (inst[l-2] == '_') and std::isdigit(inst[l-1]));
          ok &= kind != "sc_object";
          ok &= kind != "sc_method_process";
          ok &= kind != "sc_thread_process";
        }
        if( ok ) {
          MESSAGE( indent << "+- " << inst << ' ' << kind );
          //{:TODO:} Figure out and add where sc_port's are pointing
          if( channel_name.length() > 0 ) MESSAGE( " -> " << channel_name );
          MESSAGE( "\n" );
        }
        scan_hierarchy(child, indent + string("| ")); 
      }//endif
    }//endfor
  }//end scan_hierarchy()
}

Netlist::Netlist( void )
{
  // Traverse the object hierarchy below each top-level object
  std::vector<sc_object*> top_vec = sc_get_top_level_objects();
  MESSAGE( "Netlist\n{\n" );

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
//END top/main.cpp $Id$
