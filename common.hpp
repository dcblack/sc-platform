#ifndef COMMON_H
#define COMMON_H

#include <tlm>
#include <cstdint>
#include "require_cxx11.hpp"
#include "sc_cxx11.hpp"

// Useful constants
#define ns sc_core::sc_time( 1.0, sc_core::SC_NS )
#define us sc_core::sc_time( 1.0, sc_core::SC_US )
#define ms sc_core::sc_time( 1.0, sc_core::SC_MS )
#define KB 1024ull
#define MB (KB*KB)
#define GB (KB*MB)

typedef uint64_t Addr_t;
#define MAX_ADDR (~Addr_t(0))
typedef uint32_t Depth_t; //< accommodate address map
namespace { const sc_core::sc_time default_period( 10.0_ns ); }
enum class Feature { none, enabled };
using DMI = Feature;
using Byte_enables = Feature;
enum class Access { none, RW, RO, WO, RC, RS, W1C, W0C, W1S, W0S, W1T, W0T, RAZ };
enum class Style { UNKNOWN, DEFAULT, LT, AT };

#include <ostream>

std::string verbosity2str(const int & level);
std::ostream& operator<<(std::ostream& os, const Access& rhs);
std::ostream& operator<<(std::ostream& os, const Feature& rhs);
std::ostream& operator<<(std::ostream& os, const Style& rhs);

// Special globals
// TODO: Make this a singleton class with controlled accessors
extern bool  g_error_at_target;
extern Style g_coding_style;

#endif /*COMMON_H*/
