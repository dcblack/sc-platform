#!cmake .

# SystemC specific needs
set(CMAKE_PREFIX_PATH $ENV{SYSTEMC_HOME})
find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD} CACHE STRING
     "C++ standard to build all targets. Supported values are 98, 11, and 14.")
set (CMAKE_CXX_STANDARD_REQUIRED ${SystemC_CXX_STANDARD_REQUIRED} CACHE BOOL
     "The with CMAKE_CXX_STANDARD selected C++ standard is a requirement.")
link_libraries(SystemC::systemc)

# Things that affect all targets
set(shared_env LD_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib DYLD_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib)
add_custom_target(run
   env ${shared_env} ./${TARGET_EXECUTABLE} $(ARGS) 2>&1 | tee run.log
   COMMAND filter-sclog <run.log >../run.log
   DEPENDS ${TARGET_EXECUTABLE}
   COMMENT "Running program"
   USES_TERMINAL
   )
add_custom_target(debug
   env DEBUG=1 ${shared_env} ./${TARGET_EXECUTABLE} -debug -error-at-target $(ARGS) 2>&1 | tee run.log
   COMMAND filter-sclog <run.log >../run.log
   DEPENDS ${TARGET_EXECUTABLE}
   COMMENT "Debugging program"
   USES_TERMINAL
   )
add_custom_target(gdb
   env DEBUG=1 ${shared_env} gdb ./${TARGET_EXECUTABLE} -- -debug -error-at-target $(ARGS) 2>&1 | tee run.log
   COMMAND filter-sclog <run.log >../run.log
   DEPENDS ${TARGET_EXECUTABLE}
   COMMENT "Debugging program"
   USES_TERMINAL
   )

# The end
