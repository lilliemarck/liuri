add_library(liuri
  liuri.c
  liuri.h)

set_target_properties(liuri
  PROPERTIES VERSION 1.0)

install(
  TARGETS liuri
  ARCHIVE
  DESTINATION lib)
