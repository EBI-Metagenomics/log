cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(
  log
  VERSION 0.0.3
  LANGUAGES C)

# Generate compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# Set RPATH only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
     "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  # /wd5105:
  # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c5105?view=msvc-160
  set(WARNING_FLAGS /W3 /wd5105)
else()
  set(WARNING_FLAGS
      -Wall
      -Wextra
      -Wstrict-prototypes
      -Wshadow
      -Wconversion
      -Wmissing-prototypes
      -Wno-unused-parameter
      -Wsign-conversion
      -Wno-unused-function)

  if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND WARNING_FLAGS -Wno-gnu-designator -Wno-empty-translation-unit
         -Wno-gnu-statement-expression -Wno-nullability-extension)
  endif()
endif()

option(CI_BUILD "CI, extra flags will be set" OFF)
if(CI_BUILD)
  message(STATUS "CI build enabled")
  add_compile_options(-Werror)
endif()

add_library(log src/log.c)
add_library(LOG::log ALIAS log)

# merge request fix:
# https://gitlab.kitware.com/cmake/cmake/-/merge_requests/1799
if(CMAKE_VERSION VERSION_GREATER 3.12.0)
  # Hide functions by default.
  set_property(TARGET elapsed PROPERTY C_VISIBILITY_PRESET hidden)
  set_property(TARGET elapsed PROPERTY VISIBILITY_INLINES_HIDDEN ON)
else()
  # Fix for https://github.com/horta/almosthere/issues/3
  set(GCC_TOO_OLD TRUE)
endif()

include(GenerateExportHeader)
generate_export_header(
  log
  BASE_NAME
  LOG
  INCLUDE_GUARD_NAME
  LOG_EXPORT_H
  EXPORT_MACRO_NAME
  LOG_API
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/log/export.h)

target_include_directories(
  log
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(log PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(log PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_features(log PRIVATE c_std_11)
target_compile_definitions(
  log PRIVATE $<$<BOOL:${WIN32}>:_CRT_NONSTDC_NO_DEPRECATE>)
target_compile_definitions(log PRIVATE $<$<BOOL:${WIN32}>:__extension__>)
target_compile_options(log PRIVATE ${WARNING_FLAGS})
target_compile_options(log PRIVATE "-fverbose-asm")

install(
  TARGETS log
  EXPORT log-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include)
install(DIRECTORY src/log/ DESTINATION include/log)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/log/export.h DESTINATION include/log)
install(
  EXPORT log-targets
  FILE log-targets.cmake
  NAMESPACE LOG::
  DESTINATION lib/cmake/log)
include(CMakePackageConfigHelpers)
configure_package_config_file(
  log-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/log-config.cmake
  INSTALL_DESTINATION lib/cmake/log
  NO_SET_AND_CHECK_MACRO)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/log-config.cmake
        DESTINATION lib/cmake/log)

enable_testing()
add_subdirectory(test)
