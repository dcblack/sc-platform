////////////////////////////////////////////////////////////////////////////////
//
//   ####  #####  ####### ###  ####  #     #  ####                                
//  #    # #    #    #     #  #    # ##    # #    #                               
//  #    # #    #    #     #  #    # # #   # #                                    
//  #    # #####     #     #  #    # #  #  #  ####                                
//  #    # #         #     #  #    # #   # #      #                               
//  #    # #         #     #  #    # #    ## #    #                               
//   ####  #         #    ###  ####  #     #  ####                                
//
////////////////////////////////////////////////////////////////////////////////
#ifndef OPTIONS_HPP
#define OPTIONS_HPP
#include <systemc>
#include "signal.hpp"
#include <set>

enum class Interconnect 
{ DEFAULT
, MEMORY
, TIMER
, NORTH_SOUTH
, PIC
, TRIVIAL
};

enum class Test
{ DEFAULT
, MEMORY
, TIMER
, PIC
, TRIVIAL
};

// Singleton class
struct Options
{
  static Options* const instance( void );
  Interconnect get_configuration( void ) const { return m_configuration; }
  const std::set<Test>& get_test_set( void ) const { return m_test_set; }
private:

  Options( void ); //< Default constructor
  Interconnect   m_configuration;
  std::set<Test> m_test_set;
  Signal         m_interrupt{ Signal::INTERRUPT };
};

#endif /*OPTIONS_HPP*/
