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
#  if   (REQUIRES_CPP == 03)
     static_assert(__cplusplus >= 199711L);
#    ifndef CPP_VERSION
#      define CPP_VERSION 1998
#    endif
#  elif (REQUIRES_CPP == 11)
     static_assert(__cplusplus >= 201103L);
#    ifndef CPP_VERSION
#      define CPP_VERSION 2011
#    endif
#  elif (REQUIRES_CPP == 14)
     static_assert(__cplusplus >= 201402L);
#    ifndef CPP_VERSION
#      define CPP_VERSION 2014
#    endif
#  elif (REQUIRES_CPP == 17)
     static_assert(__cplusplus >= 201703L);
#    ifndef CPP_VERSION
#      define CPP_VERSION 2017
#    endif
#  else
#    error #__cplusplus does not meet minimum requirement of C++REQUIRES_CPP
#  endif
#  undef REQUIRES_CPP
#else
#  error REQUIRES_CPP not defined
#endif

// The following makes sure the appropriately specified version of SystemC is
// available. Must define REQUIRES_SYSTEMC to specify the minimum required version.
//
// Currently recognizes:
// 201 => 20020405L
// 211 => 20050714L
// 220 => 20070314L
// 230 => 20120701L
// 231 => 20140417L
// 232 => 20171012L
// 233 => 20181013L
// 234 => 20181013L
//
// Warning: Defines SYSC_VERSION only on first encounter (i.e. not defined)

#ifdef REQUIRES_SYSTEMC
#  if   (REQUIRES_SYSTEMC == 240)
     static_assert(SYSTEMC_VERSION >= 20191203L);
#    define SYSC_VERSION 2.4.0
#  elif (REQUIRES_SYSTEMC == 234)
     static_assert(systemc_version >= 20181013L);
#    define SYSC_VERSION 2.3.4
#  elif (REQUIRES_SYSTEMC == 233)
     static_assert(SYSTEMC_VERSION >= 20181013L);
#    define SYSC_VERSION 2.3.3
#  elif (REQUIRES_SYSTEMC == 232)
     static_assert(SYSTEMC_VERSION >= 20171012L);
#    define SYSC_VERSION 2.3.2
#  elif (REQUIRES_SYSTEMC == 231)
     static_assert(SYSTEMC_VERSION >= 20140417L);
#    define SYSC_VERSION 2.3.1
#  elif (REQUIRES_SYSTEMC == 230)
     static_assert(SYSTEMC_VERSION >= 20120701L);
#    define SYSC_VERSION 2.3.0
#  elif (REQUIRES_SYSTEMC == 220)
     static_assert(SYSTEMC_VERSION >= 20070314L);
#    define SYSC_VERSION 2.2.0
#  elif (REQUIRES_SYSTEMC == 211)
     static_assert(SYSTEMC_VERSION >= 20050714L);
#    define SYSC_VERSION 2.1.1
#  elif (REQUIRES_SYSTEMC == 201)
     static_assert(SYSTEMC_VERSION >= 20020405L);
#    define SYSC_VERSION 2.0.1
#  else
     static_assert( SYSTEMC_VERSION == 0, "Unknown requirement REQUIRES_SYSTEMC" );
#  endif
#else
#  error REQUIRES_SYSTEMC not defined
#endif
