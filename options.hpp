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
#include <map>

enum class Interconnect 
{ DEFAULT
, MEMORY
, TIMER
, NORTH_SOUTH
, PIC
, GPIO
, DMA
, POWER
, ANALYSIS
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
  static bool        has_flag( std::string );
  static std::string get_flag( std::string );
private:

  Options( void ); //< Default constructor
  
  // Attribbutes
  using flag_map_t = std::map<std::string, std::string>;
  Interconnect     m_configuration;
  flag_map_t       m_flag_map;
  std::set<Test>   m_test_set;
  Signal           m_interrupt{ Signal::INTERRUPT };

};

#endif /*OPTIONS_HPP*/
