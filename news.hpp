#ifndef NEWS_HPP
#define NEWS_HPP
#include <systemc>
#include <string>
#include <map>

struct News
{
  News(void);
  static void report_handler( const sc_report& the_report, const sc_actions& the_actions );
  static const std::string compose_report(const sc_report& the_report);
  static void add_tag( const std::string& tag );
  static size_t get_tag_count( const std::string& tag ) { return tag_map[tag]; }
private:
  static bool initialized;
  static std::map<std::string,size_t> tag_map;
};

#endif /*NEWS_HPP*/
