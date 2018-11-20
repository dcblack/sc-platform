#ifndef REQUIRE_CXX_VERSION_HPP
#define REQUIRE_CXX_VERSION_HPP

// The following makes sure the appropriately specified version of C++ is
// available. Must define REQUIRES_CPP to specify the minimum required version.
//
// Currently recognizes:
//   03 => 199711L
//   11 => 201103L
//   14 => 201402L
//   17 => 201703L

#ifdef  CPP_VERSION
# undef CPP_VERSION
#endif
#ifdef REQUIRES_CPP
#  if   (REQUIRES_CPP == 03) && (__cplusplus >= 199711L)
#    define CPP_VERSION 1998
#  elif (REQUIRES_CPP == 11) && (__cplusplus >= 201103L)
#    define CPP_VERSION 2011
#  elif (REQUIRES_CPP == 14) && (__cplusplus >= 201402L)
#    define CPP_VERSION 2014
#  elif (REQUIRES_CPP == 17) && (__cplusplus >= 201703L)
#    define CPP_VERSION 2017
#  else
#    define CPP_VERSION "unknown"
#    ifdef WIN32
#      pragma message(__cplusplus # "Does not meet minimum requirement of C++" # REQUIRES_CPP)
#    else
#      warning #__cplusplus does not meet minimum requirement of C++REQUIRES_CPP
#    endif
#  endif
#  undef REQUIRES_CPP
#else
#  define CPP_VERSION "unspecified"
#  ifdef WIN32
#    pragma message("error REQUIRES_CPP not defined")
#  else
#    error REQUIRES_CPP not defined
#  endif
#endif

#endif /*REQUIRE_CXX_VERSION_HPP*/
