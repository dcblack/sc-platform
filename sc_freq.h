#ifndef SC_FREQ_H
#define SC_FREQ_H

// Frequency supports:
//   f1 = f2
//   f1 + f2
//   f1 / f2
//    k * f1
//   f1 * k
//   f1 / k

#if __cplusplus >= 201103L

#include <systemc>
#include <iostream>
#include <cmath>

namespace sc_core {
enum sc_freq_units : int { SC_HZ, SC_KHZ, SC_MHZ, SC_GHZ, SC_THZ };
struct sc_freq
{

  // Constructors
  sc_freq( void ) : m_value(0.0), m_units( SC_HZ ) {}
  explicit sc_freq( double value, sc_freq_units units = SC_HZ )
  : m_value( value ), m_units( units ) {}
  sc_freq( const sc_freq& ) = default;
  sc_freq( sc_freq&& ) = default;
  ~sc_freq( void ) = default;

  // Assignment
  sc_freq& operator=( const sc_freq& rhs ) = default;
  sc_freq& operator=( sc_freq&& rhs ) = default;

  // Utility
  static double magnitude( sc_freq_units units );
  static std::string str( sc_freq_units units );

  // Accessors & conversion
  double value( sc_freq_units units ) const {
    double scale = magnitude( m_units ) / magnitude( units );
    return m_value * scale;
  }
  sc_freq normalize( void ) const {
    double  v = value( SC_HZ );
    sc_freq_units u = SC_HZ;
    if( v >= magnitude( SC_THZ ) ) return sc_freq( value( SC_THZ ), u );
    if( v >= magnitude( SC_GHZ ) ) return sc_freq( value( SC_GHZ ), u );
    if( v >= magnitude( SC_MHZ ) ) return sc_freq( value( SC_MHZ ), u );
    if( v >= magnitude( SC_KHZ ) ) return sc_freq( value( SC_KHZ ), u );
    return                                sc_freq( value(  SC_HZ ), SC_HZ );
  }
  sc_freq_units units( void ) { return m_units; }
  operator sc_time( void )
  {
    double divisor { value( SC_HZ ) };
    if( divisor == 0.0 ) {
      return sc_max_time();
    } else {
      return sc_time( abs( 1.0 / divisor ), sc_core::SC_SEC );
    }
  }
  std::string to_string( int w=0, unsigned int f=3, char c=' ' ) const {
    std::string result(std::to_string( m_value ));
    size_t p = result.find('.');
    if( p != std::string::npos ) {
      if( f > 0 and ( result.length() - p - 1 ) > 0 ) {
        result.erase( p+f, std::string::npos );
      }
    }
    if( w > 0 && result.length() < w ) {
      result.insert( 0, w - result.length(), c );
    }
    result += "_" + str( m_units );
    if( w < 0 && result.length() < -w ) {
      result.append( -w - result.length(), c );
    }
    return result;
  }

  // Arithmetic
  const sc_freq& operator+=( const sc_freq& rhs ) {
    m_value += rhs.m_value;
    return *this;
  }
  sc_freq operator+( const sc_freq& rhs ) const { return sc_freq( *this ) += rhs; }
  const sc_freq& operator-=( const sc_freq& rhs ) {
    m_value -= rhs.m_value;
    return *this;
  }
  sc_freq operator-( const sc_freq& rhs ) { return sc_freq( *this ) -= rhs; }
  sc_freq operator-( void ) { return sc_freq( *this )*-1.0; }
  const sc_freq& operator*=( double rhs )
  {
    m_value *= rhs;
    return *this;
  }
  sc_freq  operator*( double rhs ) const {
    sc_freq result( *this );
    result *= rhs;
    return result;
  }
  const sc_freq& operator/=( double rhs )
  {
    m_value /= rhs;
    return *this;
  }
  sc_freq operator/( double rhs ) const { return sc_freq( *this ) /= rhs; }

private:
  double        m_value;
  sc_freq_units m_units;
};

inline sc_freq operator*( double lhs, const sc_freq& rhs );
inline sc_time operator/( double lhs, const sc_freq& rhs );
inline double operator/( const sc_freq& lhs, const sc_freq& rhs );
std::ostream& operator<<( std::ostream& os, const sc_freq& rhs );

inline sc_freq operator "" _Hz  ( long double        val ) { return sc_freq( val          , sc_core::SC_HZ  ); }
inline sc_freq operator "" _Hz  ( unsigned long long val ) { return sc_freq( double( val ), sc_core::SC_HZ  ); }
inline sc_freq operator "" _KHz ( long double        val ) { return sc_freq( val          , sc_core::SC_KHZ ); }
inline sc_freq operator "" _KHz ( unsigned long long val ) { return sc_freq( double( val ), sc_core::SC_KHZ ); }
inline sc_freq operator "" _MHz ( long double        val ) { return sc_freq( val          , sc_core::SC_MHZ ); }
inline sc_freq operator "" _MHz ( unsigned long long val ) { return sc_freq( double( val ), sc_core::SC_MHZ ); }
inline sc_freq operator "" _GHz ( long double        val ) { return sc_freq( val          , sc_core::SC_GHZ ); }
inline sc_freq operator "" _GHz ( unsigned long long val ) { return sc_freq( double( val ), sc_core::SC_GHZ ); }
inline sc_freq operator "" _THz ( long double        val ) { return sc_freq( val          , sc_core::SC_THZ ); }
inline sc_freq operator "" _THz ( unsigned long long val ) { return sc_freq( double( val ), sc_core::SC_THZ ); }

}
#endif

#endif /*SC_FREQ_H*/
