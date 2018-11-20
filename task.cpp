#include "task.hpp"
#include "report.hpp"

namespace {
  const char* const MSGID{ "/Doulos/Example/Task_manager" };
}
using namespace std;

std::map<string,Task_manager::Task_list> Task_manager::s_tasks;

// Task_manager boss constructor
Task_manager::Task_manager(std::string boss_name, Task_obj_t* boss_ptr)
: m_boss_name(boss_name)
{
  if( boss_name == "" ) {
    REPORT( ERROR, "Empty boss name not allowed." );
  }
 if( s_tasks.count(boss_name) == 0 ) {
    s_tasks[boss_name] = Task_list{};
  }
  if ( s_tasks[boss_name].boss_ptr == nullptr ) {
    s_tasks[boss_name].boss_ptr = boss_ptr;
  }
  else {
    REPORT( ERROR, "Task lists may only have one boss" );
  }
}
// Task worker constructor
Task_manager::Task_manager(std::string boss_name, std::string task_name, Task_body_t task_body)
: m_boss_name(boss_name)
, m_task_name(task_name)
{
  if( boss_name == "" or task_name == "" ) {
    REPORT( ERROR, "Empty names not allowed." );
  }
  if( s_tasks.count(boss_name) == 0 ) {
    s_tasks[boss_name] = Task_list{};
  }
  if( s_tasks[boss_name].task.count(task_name) == 0 ) {
    s_tasks[boss_name].task[task_name] = task_body;
  }
  else {
    REPORT( ERROR, "Tasknamme already taken." );
  }
}

const Task_manager::Task_map_t& Task_manager::tasks( void )
{
  if( m_task_name.size() > 0 ) {
    REPORT( ERROR, "Only boss may access tasks." );
  }
  if( s_tasks.count(m_boss_name) == 0 ) {
    REPORT( FATAL, "Owner entry absent!" );
  }
  if( s_tasks[m_boss_name].task.empty() ) {
    REPORT( WARNING, "No tasks defined for " << m_boss_name );
  }
  return s_tasks[m_boss_name].task;
}

Task_manager::Task_obj_t* Task_manager::obj(void)
{
  Task_obj_t* result{ nullptr };
  if( s_tasks.count(m_boss_name) == 0 ) {
    REPORT( FATAL, "Missing boss '" << m_boss_name << "'" );
  }
  else {
    result = s_tasks[m_boss_name].boss_ptr;
  }
  if( result == nullptr ) {
    REPORT( FATAL, "Uninitialized boss '" << m_boss_name << "'" );
  }
  return result;
}

// The end
