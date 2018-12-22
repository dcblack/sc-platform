#ifndef NETLIST_HPP
#define NETLIST_HPP
#include <systemc>
#include <string>
#include <ostream>

struct Netlist
{
  // Gang of six
  Netlist( void ); // Constructor
  virtual ~Netlist( void ) = default; // Destructor
private:
  Netlist( const Netlist& rhs ) = delete; // Copy constructor
  Netlist( Netlist&& rhs ) = delete; // Move constructor
  Netlist& operator=( const Netlist& rhs ) = delete; // Copy assignment
  Netlist& operator=( Netlist&& rhs ) = delete; // Move assignment
};

#endif /*NETLIST_HPP*/
