#ifndef CPU_IF_HPP
#define CPU_IF_HPP
#include <systemc>
#include "common/common.hpp"

struct Cpu_if : virtual sc_core::sc_interface
{
  virtual void write32  ( Addr_t address, uint32_t data )  = 0;
  virtual void read32   ( Addr_t address, uint32_t& data ) = 0;
  virtual void write16  ( Addr_t address, uint16_t data )  = 0;
  virtual void read16   ( Addr_t address, uint16_t& data ) = 0;
  virtual void write8   ( Addr_t address, uint8_t  data )  = 0;
  virtual void read8    ( Addr_t address, uint8_t&  data ) = 0;
  virtual void write    ( Addr_t address, uint8_t* data, Depth_t length ) = 0;
  virtual void read     ( Addr_t address, uint8_t* data, Depth_t length ) = 0;
  virtual void put( Addr_t address, std::vector<uint8_t>& vec ) = 0;
  virtual void get( Addr_t address, Depth_t depth, std::vector<uint8_t>& vec ) = 0;
};
#endif /*CPU_IF_HPP*/
