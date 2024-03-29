#!cmake ..
# vim:syntax=cmake
#
# See /apps/cmake/ABOUT_CMAKE.md for instructions.
#
# This works with SystemV versions 2.3.2 and later, which have support for
# cmake and specify SystemC_CXX_STANDARD.
# 
# REQUIREMENTS:
#   Define SYSTEMC_HOME to point to install directory of SystemC (not source)
#   that has been built with CMake.

cmake_minimum_required( VERSION 3.11 )
set( TARGET_EXECUTABLE $<TARGET_FILE_NAME:platform> )
set( TARGET_NAME platform )
project( platform 
  VERSION 0.2
  DESCRIPTION "Example TLM 2.0 Platform"
  LANGUAGES CXX C
  )

include( cmake/BuildTypes.cmake )
include( cmake/SystemC.cmake )
include( cmake/strict.cmake )
find_package( yaml-cpp CONFIG REQUIRED )
find_package( Boost 1.65 REQUIRED filesystem )

add_compile_definitions( SC_INCLUDE_FX SC_INCLUDE_DYNAMIC_PROCESSES )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( . common )
#include_directories( BEFORE /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/ )

# add a target to generate API documentation with Doxygen
find_package( Doxygen REQUIRED dot )
if( DOXYGEN_FOUND )
  set( DOXYGEN_OUTPUT_DIRECTORY ../doc )
  set( DOXYGEN_IMAGE_PATH . )
  set( DOXYGEN_EXTRACT_ALL yes )
  set( DOXYGEN_EXTRACT_PRIVATE yes )
  set( DOXYGEN_EXTRACT_STATIC yes )
  set( DOXYGEN_TOC_INCLUDE_HEADINGS 1 )
  set( DOXYGEN_EXCLUDE_PATTERNS */build*/* )
  doxygen_add_docs( docs )
endif( DOXYGEN_FOUND )


################################################################################
add_executable( platform
# Support ----------------------------------------------------------------------
  common/common.cpp
  common/log2.cpp
  scx/sc_freq.cpp
  report/report.cpp
  top/wallclock.cpp
  report/summary.cpp
  top/news.cpp
  no_clock/no_clock.cpp
  cpu/task.cpp
  top/netlist.cpp
  top/platform_enum.cpp
  unit_test/test_enum.cpp
  top/options.cpp
  utility/signal.cpp
  config/configuration.cpp
  config/config_extn.cpp
  config/config_proxy.cpp
  cpu/cpuid_extn.cpp
  bus/memory_map.cpp
  timer/timer_beh.cpp
  utility/hexfile.cpp
  uart/parity.cpp
# Major modules ----------------------------------------------------------------
  top/main.cpp
  top/top.cpp
  cpu/cpu.cpp
  bus/bus.cpp
  top/stub.cpp
  memory/memory.cpp
  timer/timer.cpp
  interrupt/pic.cpp
  gpio/gpio.cpp
  gpio/rgbled.cpp
  uart/uart.cpp
# dma/dma.cpp
# mmu/mmu.cpp
# cache.cpp
# crypto.cpp
# flash.cpp
# ssd.cpp
# usb.cpp
# mouse.cpp
# keyboard.cpp
# video.cpp
# wifi.cpp
# sbus/bus.cpp
# environ.cpp
# gps.cpp
# Testing ----------------------------------------------------------------------
# program.cpp
  memory/memory_test.cpp
  timer/timer_test.cpp
)
target_link_libraries( platform yaml-cpp Boost::filesystem)

################################################################################
add_executable( config EXCLUDE_FROM_ALL
  common/common.cpp
  config/configuration.cpp
  report/report.cpp
  top/wallclock.cpp
  report/summary.cpp
  scx/sc_freq.cpp
)
target_compile_definitions( config PRIVATE CONFIG_EXAMPLE )

################################################################################
add_executable( signal EXCLUDE_FROM_ALL
  utility/signal.cpp
  report/report.cpp
  top/wallclock.cpp
  report/summary.cpp
)

target_compile_definitions( signal PRIVATE SIGNAL_EXAMPLE )

################################################################################
add_executable( payload EXCLUDE_FROM_ALL
  async/async_kind.cpp
  async/async_payload.cpp
)

target_link_libraries( payload yaml-cpp )
target_compile_definitions( payload PRIVATE ASYNC_PAYLOAD_EXAMPLE )

################################################################################
add_executable( rgbled EXCLUDE_FROM_ALL
  gpio/rgbled.cpp
  report/report.cpp
  top/wallclock.cpp
  report/summary.cpp
  no_clock/no_clock.cpp
)
target_compile_definitions( rgbled PRIVATE RGBLED_EXAMPLE )

################################################################################
add_executable( timer EXCLUDE_FROM_ALL
  timer/timer_beh.cpp
  common/common.cpp
  scx/sc_freq.cpp
  report/report.cpp
  top/wallclock.cpp
  report/summary.cpp
  no_clock/no_clock.cpp
)
target_compile_definitions( timer PRIVATE TIMER_EXAMPLE )

################################################################################
add_executable( xyaml EXCLUDE_FROM_ALL
  unit_test/xyaml.cpp
  report/report.cpp
  top/wallclock.cpp
  report/summary.cpp
  common/common.cpp
  scx/sc_freq.cpp
)
target_compile_definitions( xyaml PRIVATE YAML_EXAMPLE )
target_link_libraries( xyaml yaml-cpp )

################################################################################
add_executable( server EXCLUDE_FROM_ALL
  tcpip/server.c
)
add_executable( client EXCLUDE_FROM_ALL
  tcpip/client.c
)

################################################################################
# add_executable( map_test
#   bus/map_test.cpp
#   bus/memory_map.cpp
#   report/report.cpp
#   top/wallclock.cpp
#   report/summary.cpp
#   common/common.cpp
#   scx/sc_freq.cpp
# )
# target_link_libraries( map_test yaml-cpp )

################################################################################
add_executable( freq_test EXCLUDE_FROM_ALL
  scx/freq_test.cpp
  scx/sc_freq.cpp
)

################################################################################
# Syntax reminders
#-------------------------------------------------------------------------------
#target_compile_features( platform PUBLIC cxx_std_11 )
#target_include_directories( platform $includes )
#target_link_libraries( platform ${CMAKE_SOURCE_DIR}/lib{:YOUR_STATIC_LIB:}.a )

################################################################################
message( "   -- CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}" )
