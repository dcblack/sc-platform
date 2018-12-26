#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

// *******************************************************************
// User-defined memory manager, which maintains a pool of transactions
//
// This implementation is a singleton templated on the tlm_generic_payload.
// There is a convenience method allocate_acquire(), which performs
// initialization and an immediate acquire after allocation.
//
// To debug allocations, use the command-line switch -debug-mm
//
// Instantiate as follows:
/*
      SC_MODULE(Example) {
        SC_CTOR(Example)
        : m_mm{ Memory_manager<>::instance() }
        ...
        {
          ...
        }
        ...
      private:
        Memory_manager<>& m_mm;
      };
*/
// *******************************************************************

#include "report.hpp"
#define REQUIRES_CPP 11
#include "require_cxx_version.hpp"
#include <tlm>
#include <forward_list>

// Templated singleton class
template<typename T=tlm::tlm_generic_payload>
struct Memory_manager: tlm::tlm_mm_interface, sc_core::sc_object
{

  // Methods
  //----------------------------------------------------------------------------
  static Memory_manager& instance( void );
  //----------------------------------------------------------------------------
  T* allocate( void );
  //----------------------------------------------------------------------------
  T* allocate_acquire( void );
  //----------------------------------------------------------------------------
  T* allocate_acquire_and_set
  ( tlm::tlm_command command            = tlm::TLM_IGNORE_COMMAND
  , sc_dt::uint64    address            = ~0ull
  , unsigned char*   data_ptr           = nullptr
  , uint32_t         data_length        = 0
  , uint32_t         streaming_width    = ~0u
  , unsigned char*   byte_enable_ptr    = nullptr
  , uint32_t         byte_enable_length = 0
  );
  //----------------------------------------------------------------------------
  void set
  ( T* t
  , tlm::tlm_command command            = tlm::TLM_IGNORE_COMMAND
  , sc_dt::uint64    address            = ~0ull
  , unsigned char*   data_ptr           = nullptr
  , uint32_t         data_length        = 0
  , uint32_t         streaming_width    = ~0u
  , unsigned char*   byte_enable_ptr    = nullptr
  , uint32_t         byte_enable_length = 0
  );
  //----------------------------------------------------------------------------
  void  free( T* trans );

private:
  // Basics
  //----------------------------------------------------------------------------
  Memory_manager( void ); //< Default constructor
  virtual ~Memory_manager( void ); //< Destructor
  Memory_manager( const Memory_manager& ) = delete; // Copy constructor
  Memory_manager( Memory_manager&& ) = delete; // Move constructor
  Memory_manager& operator=( const Memory_manager& ) = delete; // Copy assignment
  Memory_manager& operator=( Memory_manager&& ) = delete; // Move assignment

  // Attributes
  //----------------------------------------------------------------------------
  int     m_count_allocated; //< debug aid
  int     m_count_created;   //< debug aid
  std::forward_list<T*> free_list;
  static constexpr char const * const MSGID{ "/Doulos/Example/Memory_manager" };
};

#include <string>
#include <typeinfo>

//------------------------------------------------------------------------------
template<typename T>
Memory_manager<T>& Memory_manager<T>::instance( void )
{
  static Memory_manager instance{};
  return instance;
}

//------------------------------------------------------------------------------
// Constructor
template<typename T>
Memory_manager<T>::Memory_manager( void )
: sc_core::sc_object{ typeid(Memory_manager<T>).name() }
, m_count_allocated { 0       }
{
  INFO( ALWAYS, "Constructed " << name() );
  sc_core::sc_report_handler::set_actions("/Doulos/Example/Memory_manager", sc_core::SC_DO_NOTHING);
  for(int i=1; i<sc_core::sc_argc(); ++i) {
    std::string arg(sc_core::sc_argv()[i]);
    if (arg == "-debug-mm") {
      sc_core::sc_report_handler::set_actions("/Doulos/Example/Memory_manager", sc_core::SC_DISPLAY|sc_core::SC_LOG);
    }
  }
}

//------------------------------------------------------------------------------
// Destructor
template<typename T>
Memory_manager<T>::~Memory_manager( void )
{
  INFO( DEBUG, "Destructor: transaction count = " << m_count_allocated );

  for( auto ptr : free_list ) {
    delete ptr;
  }
}

//------------------------------------------------------------------------------
template<typename T>
T* Memory_manager<T>::allocate_acquire( void )
{
    T* t=allocate();
    sc_assert( t!=nullptr );
    t->acquire();
    return t;
}

//------------------------------------------------------------------------------
template<typename T>
void Memory_manager<T>::set
( T* t
, tlm::tlm_command command
, sc_dt::uint64    address
, unsigned char*   data_ptr
, uint32_t         data_length
, uint32_t         streaming_width
, unsigned char*   byte_enable_ptr
, uint32_t         byte_enable_length
) {
  t->set_command            ( command            );
  t->set_address            ( address            );
  t->set_data_ptr           ( data_ptr           );
  t->set_data_length        ( data_length        );
  t->set_streaming_width    ( streaming_width    );
  t->set_byte_enable_ptr    ( byte_enable_ptr    );
  t->set_byte_enable_length ( byte_enable_length );
  t->set_dmi_allowed        ( false );
  t->set_response_status    ( tlm::TLM_INCOMPLETE_RESPONSE );
}

//------------------------------------------------------------------------------
template<typename T>
T* Memory_manager<T>::allocate_acquire_and_set
( tlm::tlm_command command
, sc_dt::uint64    address
, unsigned char*   data_ptr
, uint32_t         data_length
, uint32_t         streaming_width
, unsigned char*   byte_enable_ptr
, uint32_t         byte_enable_length
) {
  T* t{ allocate_acquire() };
  set(t, command, address, data_ptr, data_length, streaming_width, byte_enable_ptr, byte_enable_length );
  return t;
}

//------------------------------------------------------------------------------
template<typename T>
T* Memory_manager<T>::allocate()
{
  INFO( DEBUG, "allocate: transaction count = " << ++m_count_allocated );
  T* ptr;
  if (not free_list.empty() )
  {
    ptr = free_list.front();
    free_list.pop_front();
  }
  else
  {
    INFO( DEBUG, "allocate: new object created = " << ++m_count_created );
    ptr = new T(this);
  }
  return ptr;
}

//------------------------------------------------------------------------------
template<typename T>
void Memory_manager<T>::free(T* trans)
{
  INFO( DEBUG, "free: transaction count = " << --m_count_allocated );
  trans->reset(); // Delete auto extensions
  free_list.push_front( trans );
}
#undef MSGID

#endif /*MEMORY_MANAGER_HPP*/
