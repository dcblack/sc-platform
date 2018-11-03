#!cmake .
# This works with SystemV versions 2.3.2 and later, which have support for
# cmake and specify SystemC_CXX_STANDARD.
# 
# REQUIREMENTS:
#   Define SYSTEMC_HOME to point to install directory of SystemC (not source)

cmake_minimum_required(VERSION 3.11)
set(TARGET_EXECUTABLE $<TARGET_FILE_NAME:plat>)
set(TARGET_NAME plat)
project(plat CXX)

include(cmake/BuildTypes.cmake)
include(cmake/SystemC.cmake)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(Boost 1.66 REQUIRED)

add_compile_definitions(SC_INCLUDE_FX SC_INCLUDE_DYNAMIC_PROCESSES)
include_directories(${Boost_INCLUDE_DIRS})


add_executable( plat
  common.cpp
  util.cpp
  log2.cpp
  no_clock.cpp
  main.cpp
  top.cpp
  wallclock.cpp
  config.cpp
  config_extn.cpp
  config_proxy.cpp
  hexfile.cpp
  cpu.cpp
# program.cpp
  memory.cpp
  bus.cpp
  timer_beh.cpp
  timer.cpp
# pic.cpp
# gpio.cpp
# dma.cpp
# console.cpp
# mmu.cpp
# cache.cpp
# usb.cpp
# video.cpp
)

add_executable( config
  config.cpp
  common.cpp
)

target_compile_definitions( config PRIVATE CONFIG_EXAMPLE )

#target_compile_features(plat PUBLIC cxx_std_11)
#target_include_directories(plat $includes)
#target_link_libraries(plat ${CMAKE_SOURCE_DIR}/lib{:YOUR_STATIC_LIB:}.a)
