/******************************************************************************

   Include this file to guarantee the minimum versions of both C++ and SystemC
   needed to compile your code.

   Here is an example of how to require C++17 with SystemC 2.3.4

     #define REQUIRES_CPP 17
     #define REQUIRES_SC  234
     #include <require_version.hpp>

   REQUIRES_CPP currently recognizes:
  
     03 => 199711L (aka C++03)
     11 => 201103L (aka C++11)
     14 => 201402L (aka C++14)
     17 => 201703L (aka C++17)

   If successful, defines CPP_VERSION.

   REQUIRES_SYSTEMC currently recognizes:
  
     201 => 20020405L
     211 => 20050714L
     220 => 20070314L
     230 => 20120701L
     231 => 20140417L
     232 => 20171012L
     233 => 20181013L
     240 => 20191203L

   If successful, defines SYSC_VERSION.

*******************************************************************************/

// Warning: Defines CPP_VERSION only on first encounter (i.e. not defined)

#ifdef REQUIRES_CPP
#  if   (REQUIRES_CPP == 03) && (__cplusplus >= 199711L)
#    ifndef CPP_VERSION
#      define CPP_VERSION 1998
#    endif
#  elif (REQUIRES_CPP == 11) && (__cplusplus >= 201103L)
#    ifndef CPP_VERSION
#      define CPP_VERSION 2011
#    endif
#  elif (REQUIRES_CPP == 14) && (__cplusplus >= 201402L)
#    ifndef CPP_VERSION
#      define CPP_VERSION 2014
#    endif
#  elif (REQUIRES_CPP == 17) && (__cplusplus >= 201703L)
#    ifndef CPP_VERSION
#      define CPP_VERSION 2017
#    endif
#  else
#    ifndef CPP_VERSION
#      define CPP_VERSION "unknown"
#    endif
#    ifdef WIN32
#      pragma message(__cplusplus # " does not meet minimum requirement of C++" # REQUIRES_CPP)
#    else
#      warning #__cplusplus does not meet minimum requirement of C++REQUIRES_CPP
#    endif
#  endif
#  undef REQUIRES_CPP
#else
#  ifndef CPP_VERSION
#    define CPP_VERSION "unspecified"
#  endif
#  ifdef WIN32
#    pragma message("error REQUIRES_CPP not defined")
#  else
#    error REQUIRES_CPP not defined
#  endif
#endif

// Warning: Defines SYSC_VERSION only on first encounter (i.e. not defined)

#if defined(REQUIRES_SYSTEMC) && defined(SYSTEMC_VERSION)
#  if   (REQUIRES_SYSTEMC == 240) && (SYSTEMC_VERSION >= 20191203L)
#    define SYSC_VERSION 2.3.4
#  elif (REQUIRES_SYSTEMC == 233) && (SYSTEMC_VERSION >= 20181013L)
#    define SYSC_VERSION 2.3.3
#  elif (REQUIRES_SYSTEMC == 232) && (SYSTEMC_VERSION >= 20171012L)
#    define SYSC_VERSION 2.3.2
#  elif (REQUIRES_SYSTEMC == 231) && (SYSTEMC_VERSION >= 20140417L)
#    define SYSC_VERSION 2.3.1
#  elif (REQUIRES_SYSTEMC == 230) && (SYSTEMC_VERSION >= 20120701L)
#    define SYSC_VERSION 2.3.0
#  elif (REQUIRES_SYSTEMC == 220) && (SYSTEMC_VERSION >= 20070314L)
#    define SYSC_VERSION 2.2.0
#  elif (REQUIRES_SYSTEMC == 211) && (SYSTEMC_VERSION >= 20050714L)
#    define SYSC_VERSION 2.1.1
#  elif (REQUIRES_SYSTEMC == 201) && (SYSTEMC_VERSION >= 20020405L)
#    define SYSC_VERSION 2.0.1
#  else
#    define SYSC_VERSION "unknown"
#    ifdef WIN32
#      pragma message(SYSTEMC_VERSION # " does not meet minimum requirement of SystemC " # REQUIRES_SYSTEMC)
#    else
#      if CPP_VERSION >= 2011
       static_assert( false, #SYSTEMC_VERSION " does not meet minimum requirement of SystemC " #REQUIRES_SYSTEMC );
#      else
#        warning SYSTEMC_VERSION does not meet minimum requirement of SystemC REQUIRES_SYSTEMC
#      endif
#    endif
#  endif
#elif defined(SYSTEMC_VERSION)
#  ifndef SYSC_VERSION
#    define SYSC_VERSION "unspecified"
#  endif
#  ifdef WIN32
#    pragma message("error REQUIRES_SYSTEMC not defined")
#  else
#    error REQUIRES_SYSTEMC not defined
#  endif
#endif
