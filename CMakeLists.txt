cmake_minimum_required(VERSION 3.20)
project(Cycles++ VERSION 0.2.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token")
  add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(CMakeRC)
# Add everything under root_of_project/resources to the resources target
file(GLOB_RECURSE RESOURCES RELATIVE ${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/resources/*")
list(FILTER RESOURCES EXCLUDE REGEX ".*~")
#List files
message(STATUS "${CMAKE_SOURCE_DIR}")
foreach(RESOURCE ${RESOURCES})
  message(STATUS "Found resource: ${RESOURCE}")
endforeach()
cmrc_add_resource_library(resources ALIAS resources::rc NAMESPACE cycles_resources ${RESOURCES})

add_subdirectory(src)
enable_testing() # This line allows to call ctest after compilation
add_subdirectory(tests)
add_subdirectory(docs)
add_executable(client client/client_randomio.cpp)
add_executable(client1 client/client_jafar.cpp)
