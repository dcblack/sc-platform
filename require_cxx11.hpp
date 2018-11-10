#ifndef REQUIRE_CXX11_HPP
#define REQUIRE_CXX11_HPP

// The following makes sure C++11 is available. You can check HAS_CXX11
// elsewhere in your code if you need to check again with a simple #ifdef

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__) 
#define clang_version (__clang__ * 10000 + __clang_minor__ * 100 \
                     + __clang_patchlevel__) 
#ifdef _MSC_VER
#  if _MSC_VER < 1700 /* Microsoft Visual Studio 2012 */
#    pragma message("ERROR: Requires C++11 vis avi Microsoft Visual Studio 2012 or later")
#  else
#    define HAS_CXX11
#    define CPP_VERSION 2011
#  endif
#else
#  if __cplusplus < 201103L
#    ifdef WIN32
#      pragma message("ERROR: Requires C++11")
#    else
#      error "Requires C++11"
#    endif
#  else
#    define HAS_CXX11
#    define CPP_VERSION 2011
#  endif
#endif

#endif /*REQUIRE_CXX11_HPP*/
