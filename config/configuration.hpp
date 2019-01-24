#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <tlm>
#include <string>
#include <boost/any.hpp>
#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include "common/common.hpp"
#include "report/report.hpp"
#include <functional>
#include <memory>
#include <type_traits>
using sc_core::sc_time;
using sc_core::SC_ZERO_TIME;
using std::string;

struct Configuration
{

  Configuration( bool use_defaults = true ); //< Constructor

  Configuration( const Configuration& rhs ); // Copy constructor
  Configuration& operator=( const Configuration& rhs ); // Copy assignment
  virtual ~Configuration( void ) = default;
  bool operator==( const Configuration& rhs ) const;

  //----------------------------------------------------------------------------
  using Printer_t = std::function<void( std::ostream&, const boost::any& )>;
  using Equal_t   = std::function<bool( const boost::any&, const boost::any& )>;
  using Assign_t  = std::function<void( const std::string&, boost::any&, const boost::any& )>;
  struct Function_t 
  {
    Function_t
    ( Printer_t p
    , Equal_t   e
    , Assign_t  a
    )
    : printer { p }
    , equal   { e }
    , assign  { a }
    {}
    Printer_t printer;
    Equal_t   equal;
    Assign_t  assign;
  };

  void update( const Configuration& rhs );  //< Does not remove pre-existing data unless overwritten
  void erase( const string& field ); //< remove one

  void clear_data  ( void ); //< remove all data
  void clear_keys  ( void ); //< remove all field defaults
  void set_defaults( void );

  //------------------------------------------------------------------------------
  void addkey( const string& field, boost::any value, bool reqd = false );
  void delkey( const string& field );
  bool is_key( const string& field ) const;
  bool has_key( const string& field ) const;
  bool is_reqd( const string& field ) const;

  bool full( void ) const; // all keys have values
  bool empty( void ) const; //< no data set
  bool has_reqd( void ) const; //< all required keys have values
  int  count( void ) const; //< number of keys set
  int  avail( void ) const; //< number of possible keys
  friend std::ostream& operator<<( std::ostream& os, const Configuration& rhs );

  //----------------------------------------------------------------------------
  void set( const string& field, boost::any value )
  {
    // Is it legal
    if ( ( m_dflt_map.count( field ) == 1 )
         and ( m_data_map[field].empty() or ( m_data_map[field].type().hash_code() == value.type().hash_code() ) )
       )
    {
      m_data_map[field] = value;
    }
    else {
      REPORT( WARNING, "Unable to set value for " << field << " due to misspelling or type mismatch." );
    }
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void get( const string& field, T& value )
  {
    if ( m_data_map.count( field ) == 1 and m_data_map[field].type().hash_code() == typeid( T ).hash_code() ) {
      value = boost::any_cast<T>( m_data_map[field] );
    }
    else if ( m_dflt_map.count( field ) == 1 and m_dflt_map[field].type().hash_code() == typeid( T ).hash_code() ) {
      value = boost::any_cast<T>( m_dflt_map[field] );
    }
    else {
      REPORT( WARNING, "Unable to get value for " << field 
              << " due to misspelling or type mismatch." );
    }
  }

  // Static array to store special functionality
  static std::unordered_map<size_t, std::unique_ptr<Function_t>> s_function;

private:

  // Attributes
  std::unordered_map<string, boost::any>  m_data_map;
  std::unordered_map<string, boost::any>  m_dflt_map;
  std::unordered_set<string>              m_reqd_key_set;

  static constexpr char const * const MSGID{ "/Doulos/Example/Configuration" };
};

//------------------------------------------------------------------------------
template <typename T>
void config_key( Configuration& configuration, const string& field, const T& value, bool reqd = false )
{
  configuration.addkey( field, value, reqd );

  // Add functor entries as needed
  size_t type_hash = typeid(T).hash_code();
  if( Configuration::s_function.count(type_hash) == 0) {
    Configuration::s_function[type_hash] = std::make_unique<Configuration::Function_t>
    ( /*printer*/[]( std::ostream& os, const boost::any& a )
      {
        try {
          if( std::is_integral<T>::value ) {
            T ta ( boost::any_cast<T>( a ) );
            os << HEX << ta;
            os << " (" << DEC << ta << ")";
          } else if( std::is_pointer<T>::value ) {
            os << "(pointer)";
          } else {
            T ta ( boost::any_cast<T>( a ) );
            os << ta;
          }
        } catch( boost::bad_any_cast& e ) {
          const char* MSGID{ "/Doulos/configuration/printer" };
          REPORT( WARNING, "Failed cast of field" );
        }
      }
    , /*equal*/[]( const boost::any& a, const boost::any& b ) -> bool
      {
        T ta ( boost::any_cast<const T&>( a ) );
        T tb ( boost::any_cast<const T&>( b ) );
        return ta == tb;
      }
    , /*assign*/[]( const std::string& f, boost::any& a, const boost::any& b )
      {
        const char* MSGID{ "/Doulos/configuration/assign" };
        INFO( DEBUG+1, "Assigning " << f );
        try {
          a = boost::any_cast<const T&>( b );
        } catch( boost::bad_any_cast& e ) {
          REPORT( WARNING, "Failed cast of field " << f );
        }
      }
    );
  }
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const Configuration& rhs );

#endif /*CONFIGURATION_HPP*/
