#ifndef SUMMARY_HPP
#define SUMMARY_HPP

struct Summary {
  static int report( void );
  static int errors( void );
  // For tracking errors and warnings not reported with SC_REPORT_*:
  static void increment_errors(void)   { ++s_errors; }
  static void increment_warnings(void) { ++s_warnings; }
  // For establishing timing points
  static void starting_elaboration( void );
  static void starting_simulation( void );
  static void finished_simulation( void );
private:
  static double s_elaboration_time;
  static double s_starting_time;
  static double s_finished_time;
  static unsigned int s_errors;
  static unsigned int s_warnings;
};

#endif /*SUMMARY_HPP*/
