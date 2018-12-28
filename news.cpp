#include "news.hpp"
#include "report.hpp"
#include <iostream>
using namespace std;
using namespace sc_core;

namespace {
  constexpr char const * const MSGID { "/Doulos/Example/News" };
};

bool News::initialized = false;

News::News( void )
{
  if( initialized ) return;
  sc_report_handler::set_handler( report_handler );
  initialized = true;
}

void News::report_handler( const sc_report& the_report, const sc_actions& the_actions )
{
  sc_actions new_actions( the_actions );

  if ( the_actions & SC_DISPLAY ) {
    cout << compose_report( the_report ) << endl;
    new_actions &= ~SC_DISPLAY;
  }

#if TO_BE_SUPPLIED
  if ( ( the_actions & SC_LOG ) && get_log_file_name() ) {
    log_stream.update_file_name( get_log_file_name() );
    *log_stream << the_report.get_time() << ": "
                << compose_report( the_report ) << endl;
    new_actions &= ~SC_LOG;
  }
#endif

  sc_report_handler::default_handler( the_report, new_actions );

}//end News::report_handler

std::map<string,size_t> News::tag_map =
{ {"Debug",0}
, {"Alert",0}
, {"Note",0}
, {"Data",0}
};

void News::add_tag( const std::string& tag )
{
  if( tag_map.count(tag) != 0 ) {
    REPORT( FATAL, "Tag '" << tag << "' already exists!" );
  }
  tag_map[ tag ] = 0;
}

size_t News::get_tag_count( const std::string& tag )
{
  if( tag_map.count(tag) == 0 ) {
    REPORT( FATAL, "No such tag '" << tag << "' -- did you forget to call News::add_tag()?" );
  }
  return tag_map[tag];
}

const string News::compose_report( const sc_report& the_report )
{
  static const char* severity_names[] = {
    "Info", "Warning", "Error", "Fatal"
  };
  string result;
  string msg = the_report.get_msg();

  if( the_report.get_severity() == SC_INFO ) {
    size_t pos = msg.find_first_of(':');
    string tag = msg.substr(0,pos);
    if( tag_map.count(tag) == 1 ) {
      ++tag_map[tag];
      result = tag;
      msg.erase(0,pos);
    } else {
      result += "Info";
    }
  }
  else {
    result += severity_names[the_report.get_severity()];
  }
  result += ": ";

  if ( the_report.get_id() >= 0 ) { // backward compatibility with 2.0+
    result += string( "(" ) + "IWEF"[the_report.get_severity()];
    result += to_string( the_report.get_id() ) + ")";
  }

  result += the_report.get_msg_type();

  if ( *the_report.get_msg() ) {
    result += string( ": " ) + the_report.get_msg();
  }

  if ( the_report.get_severity() > SC_INFO ) {
    result += string( "\nIn file: " ) + the_report.get_file_name();
    result += string( ":" ) + to_string( the_report.get_line_number() );

    if ( sc_get_current_process_handle().valid() && sc_is_running() ) {
      const char* proc_name = the_report.get_process_name();

      if ( proc_name ) {
        result += string("\nIn process: ") + proc_name;
        result += string(" @ ") + the_report.get_time().to_string();
      }
    }
  } else if ( the_report.get_severity() == SC_INFO ) {
    result += string(" @ ") + the_report.get_time().to_string();
  }

  return result;
}//end News::compose_report

///////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END main.cpp $Id$
