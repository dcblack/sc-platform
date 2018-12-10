// The following makes sure the appropriately specified version of C++ is
// available. Must define REQUIRES_CPP to specify the minimum required version.
//
// Currently recognizes:
//   03 => 199711L
//   11 => 201103L
//   14 => 201402L
//   17 => 201703L
//
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
#      pragma message(__cplusplus # "Does not meet minimum requirement of C++" # REQUIRES_CPP)
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
