set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")

include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

add_executable(liuri_test
  liuri_test.c)

target_link_libraries(liuri_test
  liuri
  ${CHECK_LIBRARIES})

add_test(liuri_test liuri_test)
