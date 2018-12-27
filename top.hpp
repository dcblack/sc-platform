#ifndef TOP_H
#define TOP_H
// -*- C++ -*- vim600:syntax=cpp:sw=2:tw=78
// Copyright 2018 by Doulos. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
//
//  #######  ####  #####                                                          
//     #    #    # #    #                                                         
//     #    #    # #    #                                                         
//     #    #    # #####                                                          
//     #    #    # #                                                              
//     #    #    # #                                                              
//     #     ####  #                                                              
//
////////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION
//! \brief Top level connectivity for platform.
//
// This is the top-level connectivity for a model virtual
// platform. The following is the completed platform vision.
//
// See README.md for more information.
//
////////////////////////////////////////////////////////////////////////////>>/
#include <systemc>
#include <memory>

struct Top_module
: sc_core::sc_module
{
  // Constructor
  Top_module(sc_core::sc_module_name instance_name);
  ~Top_module(void); // Destructor
  const char* kind( void ) const override { return "Top_module"; }
  void end_of_elaboration( void ) override;
  void start_of_simulation( void ) override;
  void end_of_simulation( void ) override;
  struct Impl;
  std::unique_ptr<Impl> pImpl;

  // Processes - NONE

};//end Top_module
#endif/*TOP_H*/
///////////////////////////////////////////////////////////////////////////////
//END $Id$
