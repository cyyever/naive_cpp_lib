cmake_minimum_required(VERSION 3.13)

project(MyCXXLib LANGUAGES CXX)

include(cmake/all.cmake)

# test
option(BUILD_TEST "Build test" ON)

option(BUILD_FUZZING "Build fuzzing" OFF)

set(submodule_names log util hardware)
set(submodule_libs)

include_directories(${CMAKE_SOURCE_DIR})
foreach(submodule_name IN LISTS submodule_names)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${submodule_name})
  list(APPEND submodule_libs my_cxx_lib_${submodule_name})

  install(
    DIRECTORY ${submodule_name}/src/
    DESTINATION include/cyy/cpp_lib/${submodule_name}
    FILES_MATCHING
    PATTERN "*.hpp")
endforeach()

install(
  TARGETS ${submodule_libs}
  EXPORT MyCXXLib
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib COMPONENT library
  INCLUDES
  DESTINATION include)

install(
  EXPORT MyCXXLib
  DESTINATION lib/cmake/${PROJECT_NAME}
  NAMESPACE MyCXXLib)
