#ifndef SC_CXX11_HPP
#define SC_CXX11_HPP

#include <systemc>
#include "sc_time_literal.h"
#include "sc_freq.h"

#if __cplusplus >= 201103L

// The following allows for easy bigint's: 123456789ABCDEF0123456789_BIGINT
#ifndef SC_MAX_NBITS
#define SC_MAX_NBITS 17*30
#endif
sc_dt::sc_bigint<SC_MAX_NBITS> operator "" _BIGINT(const char* literal_string);
sc_dt::sc_bigint<SC_MAX_NBITS> operator "" _BIGUINT(const char* literal_string);
#endif

#endif
