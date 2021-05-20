cmake_minimum_required(VERSION 3.0.2)
project(cee-utils)

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_C_FLAGS "-DLIBDISCORD_DEBUG -D_XOPEN_SOURCE=700 -D_DISCORD_ADD_ONS -DBEARSSL")
set(CMAKE_EXE_LINKER_FLAGS "-static")
include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SRC_CODE "*.h" "*.c")

add_library(cee-utils ${SRC_CODE})


set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

list(APPEND TESTS
  test-json-scanf.c
  test-json-scanf-array.c
  test-json-string.c
  test-json-injector.c
  test-json-injector-1.c
  test-json-actor.c
  test-json-extractor.c
  test-utf8.c
  test-ntl.c
  test-json-struct.c
  test-spec.c)

foreach(_t ${TESTS})
  list(APPEND TEST_SRC  ${TEST_DIR}/${_t})
  add_executable(${_t}.exe ${TEST_DIR}/${_t})
  target_link_libraries(${_t}.exe cee-utils)
  add_test(
    NAME ${_t}.exe
    WORKING_DIRECTORY ${TEST_DIR}
    COMMAND ${_t}.exe)
endforeach()
