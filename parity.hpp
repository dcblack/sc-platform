#ifndef PARITY_HPP
#define PARITY_HPP

#include <cstdint>
#include <cctype>

bool odd( uint8_t octet );
bool odd( uint8_t const * const data, size_t depth );
inline bool even( uint8_t octet ) { return not odd( octet ); }
inline bool even( uint8_t const * const data, size_t depth ) { return not odd( data, depth ); }

#endif /*PARITY_HPP*/
