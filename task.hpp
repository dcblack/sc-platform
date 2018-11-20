#ifndef TASK_HPP
#define TASK_HPP

#include <systemc>
#include <string>
#include <map>

struct Task_manager
{
  using Task_body_t = void (*)(void);
  using Task_obj_t  = sc_core::sc_object;
  using Task_map_t  = std::map<std::string,Task_body_t>;

  struct Task_list
  {
    Task_obj_t* boss_ptr{nullptr};
    Task_map_t  task;
  };

  // Task boss constructor
  Task_manager(std::string boss_name, Task_obj_t* boss_ptr);
  // Task worker constructor
  Task_manager(std::string boss_name, std::string task_name, Task_body_t task_body);
  const Task_map_t& tasks( void );
  Task_obj_t* obj(void);

private:
  std::string m_boss_name;
  std::string m_task_name;
  static std::map<std::string,Task_list> s_tasks;
};

#endif /*TASK_HPP*/
