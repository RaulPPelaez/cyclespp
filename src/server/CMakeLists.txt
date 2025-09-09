include(FetchContent)

set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)

add_library(game_logic OBJECT game_logic.cpp)
add_library(configuration OBJECT configuration.cpp)
add_library(renderer OBJECT renderer.cpp)
target_link_libraries(configuration PUBLIC yaml-cpp::yaml-cpp)

add_executable(server server.cpp)
target_link_libraries(server PUBLIC game_logic configuration renderer)
target_link_libraries(renderer PRIVATE resources::rc)
