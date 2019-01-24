#include "utility/hexfile.hpp"
#include "report/report.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
using namespace std;

namespace hexfile {

//------------------------------------------------------------------------------
// Utilities
namespace {

  char const * const MSGID{ "/Doulos/Example/HexFile" };
  const char* whitespace{ " \t\n\r" };
  const char* hexdigits { "0123456789ABCDEFabcdef" };
  const int aw = 8;

  //----------------------------------------------------------------------------
  void strip_leading( string& s ) {
    size_t pos = s.find_first_not_of( whitespace );
    if ( pos == string::npos ) {
      s = "";
    } else if ( pos > 0 ) {
      s.erase( 0, pos );
    }
  }

  //----------------------------------------------------------------------------
  void strip_trailing( string& s ) {
    size_t pos = s.find_last_not_of( whitespace );
    if ( pos == string::npos ) {
      s = "";
    } else if ( (pos+1) < s.size() ) {
      s.erase( pos+1 );
    }
  }

  //----------------------------------------------------------------------------
  void strip_underscores( string& s ) {
    size_t pos{0};
    while (pos < s.size() ) {
      if ( s[pos] == '_' ) {
        s.erase(pos,1);
      }
      else {
        ++pos;
      }
    }//endwhile
  }

}//endnamespace

//------------------------------------------------------------------------------
void
dump( Addr_t address, uint8_t* data_ptr, Depth_t data_len, string name )
{
  MESSAGE("\n");
  RULER('-');
  MESSAGE("Dump " << name << hex);

  const char* group_separator{" "};
  const int bytes_per_line{16};
  const int bytes_per_group{2};

  for(Addr_t offset = 0; offset < data_len; ++offset) {
    if ( (offset % bytes_per_line) == 0 ) {
      MESSAGE( "\n  0x" << setfill('0') << setw(aw) << (address + offset) << ": " );
    }
    MESSAGE( setfill('0') << setw(2) << noshowbase << int(data_ptr[offset]) );
    if ( (offset % bytes_per_group) == (bytes_per_group - 1) ) {
      MESSAGE(group_separator);
    }
  }
  MESSAGE("\n");
  MEND(MEDIUM);
}

//------------------------------------------------------------------------------
void
save(const string& vhex_filename, Addr_t address, uint8_t* data_ptr, Depth_t data_len )
{
  ofstream ofile{ vhex_filename };
  const int bytes_per_line{16};

  if ( not ofile ) {
    REPORT( ERROR, "Unable to write '" << vhex_filename << "'" );
  }

  ofile << "// " << vhex_filename << "\n"
        << "\n"
        << "@" << setfill('0') << setw(aw) << hex << address << "\n"
        ;

  for( size_t i = 0; i < data_len; ++i ) {
    ofile << setfill('0') << setw(2) << hex << int(data_ptr[i]);
    ofile << (( ( i % bytes_per_line ) == ( bytes_per_line - 1) ) ? "\n" : " ");
  }

  ofile << "\n// " << dec << " bytes\n";

  ofile << "\n// End of file" << endl;

  ofile.close();
}

//------------------------------------------------------------------------------
Addr_t
load(const string& vhex_filename, vector<uint8_t>& mem )
{
  Addr_t address{0};
  Addr_t offset{MAX_ADDR};
  uint32_t d;
  ifstream ifile{ vhex_filename };

  if ( not ifile ) {
    REPORT( ERROR, "Unable to read '" << vhex_filename << "'" );
    return offset;
  }

  string line;
  string token;
  int byte;
  size_t lineno{0U};

  while ( not ifile.eof() ) {
    //Read an input line
    std::getline( ifile, line );
    ++lineno;

    // Strip comments
    {
      size_t pos = line.find( "//" );
      if ( pos != string::npos ) {
        line.erase( pos );
      }
    }
    strip_leading(line);
    strip_underscores(line);
    strip_trailing(line);

    if ( line.empty() ) {
      continue;
    }

    // Assert: line contains one or more tokens

    // Process the line one token at a time
    istringstream line_iss( line );
    while( line_iss >> token ) {

      if ( token[0] == '@' ) {
        address = stoull( token.substr( 1 ), nullptr, 16 );
      }
      else if ( token.find_first_of( hexdigits ) == 0 ) {
        // Verify they're all hex digits
        if ( token.find_first_not_of( hexdigits ) != string::npos ) {
          REPORT(ERROR,"Bad input on line " << lineno);
        }
        // Make sure its an even number of hex characters
        if ( (token.size() & 1) == 1 ) {
          token.insert( 0, "0" );
        }
        // For every two hex characters
        for ( int i = 0; i < token.size(); i += 2 ) {
          // Set offset if unset
          if ( offset == MAX_ADDR ) {
            offset = address;
          }
          // Make sure there is room in the vector
          if ( (address-offset+1) >= mem.size() ) {
            size_t new_size{ mem.size() + std::min( size_t(1024 * 1024), size_t(mem.size()) / 2 ) };
            mem.reserve( new_size );
            mem.resize( (address-offset+1) );
          }//endif

          // Convert one byte
          mem.at( address-offset ) = stoi( token.substr( i, 2 ), nullptr, 16 );
          ++address;
        }//endfor
      }
      else {
        REPORT(ERROR,"Bad input on line " << lineno);
      }
    }//endwhile

  }//endwhile

  ifile.close();

  return offset;
}

}//endnamespace hexfile

////////////////////////////////////////////////////////////////////////////////
// Copyright 2018 by Doulos. All rights reserved.
//END $Id$
