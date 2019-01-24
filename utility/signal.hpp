#ifndef SIGNAL_HPP
#define SIGNAL_HPP
////////////////////////////////////////////////////////////////////////////////
//
//   ####  ###  ####  #     #    #    #                                           
//  #    #  #  #    # ##    #   # #   #                                           
//  #       #  #      # #   #  #   #  #                                           
//   ####   #  #  ### #  #  # #     # #                                           
//       #  #  #    # #   # # ####### #                                           
//  #    #  #  #    # #    ## #     # #                                           
//   ####  ###  ####  #     # #     # #####                                       
//
////////////////////////////////////////////////////////////////////////////////
#include <signal.h>
#include <forward_list>

struct Signal {
  enum { STOP=-1, ABORT=-2, INTERRUPT=-3 };
  using Handler_t = void (*)( int );
  Signal( int sig, Handler_t sighandler = nullptr );
  ~Signal( void );
  void install( void );
  void remove( void );
private:
  void revert( void );
  void add( int sig, Handler_t sighandler );
  int                    m_sig;
  Handler_t              m_sighandler{ nullptr };
  std::forward_list<int> m_siglist;
  static std::forward_list<Signal*> s_handlers;
};

#endif /*SIGNAL_HPP*/
