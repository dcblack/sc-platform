#!cmake .
# This works with SystemV versions 2.3.2 and later, which have support for
# cmake and specify SystemC_CXX_STANDARD.
# 
# REQUIREMENTS:
#   Define SYSTEMC_HOME to point to install directory of SystemC (not source)

cmake_minimum_required(VERSION 3.11)
set(TARGET_EXECUTABLE $<TARGET_FILE_NAME:platform>)
set(TARGET_NAME platform)
project(platform CXX)

include(cmake/BuildTypes.cmake)
include(cmake/SystemC.cmake)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(Boost 1.65 REQUIRED filesystem)

add_compile_definitions(SC_INCLUDE_FX SC_INCLUDE_DYNAMIC_PROCESSES)
include_directories(${Boost_INCLUDE_DIRS})

################################################################################
add_executable( platform
  common.cpp
  sc_freq.cpp
  log2.cpp
  report.cpp
  wallclock.cpp
  summary.cpp
  no_clock.cpp
  task.cpp
  main.cpp
  top.cpp
  signal.cpp
  configuration.cpp
  config_extn.cpp
  config_proxy.cpp
  hexfile.cpp
  memory_map.cpp
  options.cpp
  cpu.cpp
# program.cpp
  stub.cpp
  memory.cpp
  memory_test.cpp
  bus.cpp
  timer_beh.cpp
  timer.cpp
  timer_test.cpp
# pic.cpp
# gpio.cpp
# dma.cpp
# mmu.cpp
# cache.cpp
# crypto.cpp
# flash.cpp
# ssd.cpp
# usb.cpp
# mouse.cpp
# keyboard.cpp
# usart.cpp
# video.cpp
# wifi.cpp
# sbus.cpp
# environ.cpp
# gps.cpp
)
target_link_libraries( platform yaml-cpp Boost::filesystem)

################################################################################
add_executable( config
  configuration.cpp
  report.cpp
  wallclock.cpp
  summary.cpp
  common.cpp
  sc_freq.cpp
)
target_compile_definitions( config PRIVATE CONFIG_EXAMPLE )

################################################################################
add_executable( signal
  signal.cpp
  report.cpp
  wallclock.cpp
  summary.cpp
)

target_compile_definitions( signal PRIVATE SIGNAL_EXAMPLE )

################################################################################
add_executable( timer
  timer_beh.cpp
  common.cpp
  sc_freq.cpp
  report.cpp
  wallclock.cpp
  summary.cpp
  no_clock.cpp
)
target_compile_definitions( timer PRIVATE TIMER_EXAMPLE )

################################################################################
add_executable( xyaml
  xyaml.cpp
  report.cpp
  wallclock.cpp
  summary.cpp
  common.cpp
  sc_freq.cpp
)
target_compile_definitions( xyaml PRIVATE YAML_EXAMPLE )
target_link_libraries( xyaml yaml-cpp )

################################################################################
add_executable( map_test
  map_test.cpp
  memory_map.cpp
  report.cpp
  wallclock.cpp
  summary.cpp
  common.cpp
  sc_freq.cpp
)
target_link_libraries( map_test yaml-cpp )

################################################################################
add_executable( freq_test
  freq_test.cpp
)

################################################################################
# Syntax reminders
#target_compile_features(platform PUBLIC cxx_std_11)
#target_include_directories(platform $includes)
#target_link_libraries(platform ${CMAKE_SOURCE_DIR}/lib{:YOUR_STATIC_LIB:}.a)

################################################################################
message("   -- CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
