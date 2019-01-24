#include "tcpip_rx.hpp"
#include "report.hpp" //{:DELETE THIS COMMENT, AND POSSIBLY THE LINE AS NEEDED:}
using namespace sc_core;

//------------------------------------------------------------------------------
// Constructor
Tcpip_rx_channel::Tcpip_rx_channel(sc_module_name instance_name)
: sc_channel(instance_name)
//, {:OBJECT_CONSTRUCTION:}
{
  // Connectivity - NONE
  // Register processes
  SC_HAS_PROCESS(Tcpip_rx_channel);
  SC_THREAD(tcpip_rx_thread);
}//endconstructor

//------------------------------------------------------------------------------
// Destructor
Tcpip_rx_channel::~Tcpip_rx_channel( void ) {
}

//------------------------------------------------------------------------------
// Processes
void Tcpip_rx_channel::tcpip_rx_thread( void ) {

  //(/* - vim users place cursor on this line and type 0wd%
{:*/  SC_REPORT_ERROR(MSGID,"REPLACE_HERE_BELOW\n\n\n"                 //:}*/
{:*/                                                                   //:}*/
{:*/      "########################################################\n" //:}*/
{:*/      "########################################################\n" //:}*/
{:*/      "##                                                    ##\n" //:}*/
{:*/      "##   REPLACE THIS ERROR MESSAGE AND ANY OTHER         ##\n" //:}*/
{:*/      "##   PLACEHOLDERS SURROUNDED BY CURLY BRACKET-COLON   ##\n" //:}*/
{:*/      "##   PAIRS SUCH AS YOU SEE ON THESE LINES OF SOURCE   ##\n" //:}*/
{:*/      "##   CODE. DELETE UNUSED PLACEHOLDERS ENGTIRELY.      ##\n" //:}*/
{:*/      "##                                                    ##\n" //:}*/
{:*/      "########################################################\n" //:}*/
{:*/      "########################################################\n" //:}*/
{:*/                                                                   //:}*/
{:*/  "\n\nREPLACE_HERE_ABOVE\n\n" );                                  //:}*/
//)

}//end Tcpip_rx_channel::tcpip_rx_thread

//------------------------------------------------------------------------------
// Other methods
// -none-

//------------------------------------------------------------------------------
// Copyright 2019 by Doulos. All rights reserved.
// For licensing information concerning this document see LICENSE-{:LICENSE:}.txt.
//END tcpip_rx.cpp @(#)$Id$
