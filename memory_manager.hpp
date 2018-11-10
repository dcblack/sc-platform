#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

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

template<typename T=tlm::tlm_generic_payload>
struct Memory_manager: tlm::tlm_mm_interface, sc_core::sc_object
{

  // Methods
  static Memory_manager& instance( void );
  T* allocate( void );
  T* allocate_acquire( void );
  void  free( T* trans );

private:
  Memory_manager( void ); //< Constructor
  virtual ~Memory_manager( void ); //< Destructor
  Memory_manager( const Memory_manager& ) = delete;
  Memory_manager( Memory_manager&& ) = delete;
  Memory_manager& operator=( const Memory_manager& ) = delete;
  Memory_manager& operator=( Memory_manager&& ) = delete;
  // Local classes
  struct Access_t
  {
    T* trans;
    Access_t* next;
    Access_t* prev;
  };
  // Attributes
  int     m_count_allocated;
  int     m_count_created;
  Access_t* free_list;
  Access_t* empties;
};

#define MSGID "/Doulos/Example/Memory_manager"
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
, free_list         { nullptr }
, empties           { nullptr }
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
  T* ptr;

  while (free_list)
  {
    ptr = free_list->trans;

    // Delete generic payload and all extensions
    sc_assert(ptr);
    delete ptr;

    Access_t* x = free_list;
    free_list = free_list->next;
    
    // Delete free list Access_t struct
    delete x;
  }

  while (empties)
  {
    Access_t* x = empties;
    empties = empties->prev;

    // Delete free list access struct
    delete x;
  }
}

//------------------------------------------------------------------------------
template<typename T>
T* Memory_manager<T>::allocate_acquire( void )
{
    T* t=allocate();
    sc_assert( t!=nullptr );
    t->acquire();
    t->set_address( ~0ull );           //< preventative
    t->set_data_ptr( nullptr );        //< preventative
    t->set_data_length( 0 );           //< preventative
    t->set_streaming_width( ~0u );     //< maximum
    t->set_byte_enable_ptr( nullptr );
    t->set_byte_enable_length( 0 );    //< preventative
    t->set_dmi_allowed( false );
    t->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
    return t;
}

//------------------------------------------------------------------------------
template<typename T>
T* Memory_manager<T>::allocate()
{
  INFO( DEBUG, "allocate: transaction count = " << ++m_count_allocated );
  T* ptr;
  if (free_list)
  {
    ptr = free_list->trans;
    empties = free_list;
    free_list = free_list->next;
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
  if (!empties)
  {
    empties = new Access_t;
    empties->next = free_list;
    empties->prev = nullptr;
    if (free_list)
      free_list->prev = empties;
  }
  free_list = empties;
  free_list->trans = trans;
  empties = free_list->prev;
}
#undef MSGID

#endif /*MEMORY_MANAGER_H*/
