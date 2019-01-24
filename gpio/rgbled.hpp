#ifndef RGBLED_HPP
#define RGBLED_HPP

#include <systemc>

struct RgbLED_module
: sc_core::sc_module
{
  // Ports
  sc_core::sc_in_resolved r, g, b;

  // Constructor
  RgbLED_module( sc_core::sc_module_name instance_name );

  // Destructor
  virtual ~RgbLED_module( void );

  // Processes
  void rgbled_method( void );

private:
  // Local methods - NONE

  // Local channels

  // Attributes - NONE

};//end RgbLED_module

#endif/*RGBLED_HPP*/
