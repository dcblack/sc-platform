#ifndef ANSI_HPP
#define ANSI_HPP

struct Ansi
{
  static constexpr char const * const reset      { "\033[00m" };
  static constexpr char const * const bright     { "\033[01m" };
  static constexpr char const * const dim        { "\033[02m" };
  static constexpr char const * const bold       { "\033[03m" };
  static constexpr char const * const underline  { "\033[04m" };
  static constexpr char const * const blink      { "\033[05m" };
  static constexpr char const * const reverse    { "\033[06m" };
  static constexpr char const * const hide       { "\033[07m" };
  static constexpr char const * const fg_black   { "\033[30m" };
  static constexpr char const * const fg_red     { "\033[31m" };
  static constexpr char const * const fg_green   { "\033[32m" };
  static constexpr char const * const fg_yellow  { "\033[33m" };
  static constexpr char const * const fg_blue    { "\033[34m" };
  static constexpr char const * const fg_magenta { "\033[35m" };
  static constexpr char const * const fg_cyan    { "\033[36m" };
  static constexpr char const * const fg_white   { "\033[37m" };
  static constexpr char const * const bg_black   { "\033[40m" };
  static constexpr char const * const bg_red     { "\033[41m" };
  static constexpr char const * const bg_green   { "\033[42m" };
  static constexpr char const * const bg_yellow  { "\033[43m" };
  static constexpr char const * const bg_blue    { "\033[44m" };
  static constexpr char const * const bg_magenta { "\033[45m" };
  static constexpr char const * const bg_cyan    { "\033[46m" };
  static constexpr char const * const bg_white   { "\033[47m" };
  static constexpr char const * const cursor_up  { "\033[1A"  };
  static constexpr char const * const cursor_dn  { "\033[1B"  };
  static constexpr char const * const cursor_rt  { "\033[1C"  };
  static constexpr char const * const cursor_lf  { "\033[1D"  };
};

#endif /*ANSI_HPP*/
