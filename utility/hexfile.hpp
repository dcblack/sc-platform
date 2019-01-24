#ifndef HEXFILE_H
#define HEXFILE_H

#include "common/common.hpp"

// Utilities to read, write and display Verilog writememh files

namespace hexfile {

// Display a vector as memory space with the specified starting address
void dump( Addr_t address, uint8_t* data_ptr, Depth_t data_len, const std::string name="" );
template<typename T>
void dump( Addr_t address, std::vector<T>& mem, const std::string& name="" )
{
  dump( address, reinterpret_cast<uint8_t*>(mem.data()), mem.size()*sizeof(T), name );
}

// Effectively Verilog $readmemh
// Return: Address represented by first byte (vec[0])
Addr_t load(const std::string& vhex_filename, std::vector<uint8_t>& mem );

// Effectively Verilog $writememh
void save(const std::string& vhex_filename, Addr_t address, uint8_t* data_ptr, Depth_t data_len );
template<typename T>
void save(const std::string& vhex_filename, Addr_t address, std::vector<T>& mem )
{
  save( vhex_filename, address, reinterpret_cast<uint8_t*>(mem.data()), mem.size()*sizeof(T) );
}

}

#endif /*HEXFILE_H*/
