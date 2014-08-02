find_package(PkgConfig)
pkg_check_modules(CHECK check)

if(NOT CHECK_FOUND)
	message(STATUS "Check library not found.")
	return()
endif()

include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

add_executable(liuri_test
	liuri_test.c)

target_link_libraries(liuri_test
	liuri
	${CHECK_LIBRARIES})
