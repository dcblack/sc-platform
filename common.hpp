#ifndef COMMON_HPP
#define COMMON_HPP

#include <tlm>
#include <cstdint>

#define REQUIRES_CPP 14
#include "require_cxx_version.hpp"
#include "sc_cxx11.hpp"

// Useful constants
#define KB         1024ull
#define MB         ( KB*KB )
#define GB         ( KB*MB )
#define MAX_ADDR   ( ~Addr_t(0)  )
#define BAD_ADDR   ( MAX_ADDR-1  )
#define BAD_DEPTH  ( ~Depth_t(0) )
#define BAD_PORT   ( ~Port_t(0)  )

using Addr_t  = sc_dt::uint64;
using Depth_t = uint32_t; //< accommodate address map
using Port_t = int; //< for port/socket identification
using byte_t  = uint8_t;

// Following are less common, but useful
enum class Feature { none, enabled };
using DMI = Feature;
using Byte_enables = Feature;
enum class Access { none, RW, RO, WO, RC, RS, W1C, W0C, W1S, W0S, W1T, W0T, RAZ };
enum class Style
{ UNKNOWN
, DEFAULT
, LT // Loosely Timed
, AT // Approximately Timed
, TD // LT with Temporally Decoupling
};

#include <ostream>

std::ostream& operator<<(std::ostream& os, const Access& rhs);
std::ostream& operator<<(std::ostream& os, const Feature& rhs);
std::ostream& operator<<(std::ostream& os, const Style& rhs);

// Special globals
// TODO: Make this a singleton class with controlled accessors or use CCI
extern bool  g_error_at_target;
extern Style g_coding_style;

#endif /*COMMON_HPP*/
