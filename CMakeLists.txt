cmake_minimum_required(VERSION 2.8)
project(liuri)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic")
include_directories(src)
add_subdirectory(src)

# Generate man page
add_custom_command(
  OUTPUT liuri_parse.3
  COMMAND a2x --format manpage --no-xmllint --destination-dir ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/README.adoc
  DEPENDS README.adoc)

add_custom_target(man ALL
  DEPENDS ${CMAKE_BINARY_DIR}/liuri_parse.3)

install(
  FILES ${CMAKE_BINARY_DIR}/liuri_parse.3
  DESTINATION share/man/man3)

# Add tests if check is available
find_package(PkgConfig)
pkg_check_modules(CHECK check)

if(CHECK_FOUND)
  enable_testing()
  add_subdirectory(test)
else()
  message(STATUS "Check library not found.")
endif()
