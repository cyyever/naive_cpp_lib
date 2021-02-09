cmake_minimum_required(VERSION 3.18)

project(CyyNaiveLib LANGUAGES CXX)

include(cmake/all.cmake)

set(submodule_names log util hardware data_structure task)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # list(APPEND submodule_names process)
endif()
option(BUILD_TORCH "Build torch module" OFF)
if(BUILD_TORCH)
  list(APPEND submodule_names torch)
endif()
option(BUILD_CV "Build opencv module" OFF)
option(BUILD_VIDEO "Build video modules" ON)
if(BUILD_VIDEO)
  set(BUILD_CV ON)
endif()
if(BUILD_CV)
  list(APPEND submodule_names cv)
endif()

if(BUILD_VIDEO)
  list(APPEND submodule_names video)
endif()

option(BUILD_TORCH_PYTHON_BINDING "Build python bindings for torch" ON)
option(BUILD_VIDEO_PYTHON_BINDING "Build python bindings for video" ON)

if(BUILD_TORCH_PYTHON_BINDING OR BUILD_VIDEO_PYTHON_BINDING)
  list(APPEND submodule_names python_binding)
endif()

option(WITH_ASAN "Build with address sanitizer" OFF)

option(BUILD_FUZZING "Build fuzzing" OFF)

# include_directories(${CMAKE_SOURCE_DIR})
set(submodule_libs)

foreach(submodule_name IN LISTS submodule_names)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${submodule_name})
  set(lib_name cyy_naive_lib_${submodule_name})
  if(submodule_name STREQUAL "python_binding")
    set(lib_name cyy_naive_cpp_extension)
  endif()
  if(WITH_ASAN)
    find_package(GoogleSanitizer)
    target_link_libraries(${lib_name} PUBLIC GoogleSanitizer::address)
  endif()
  if(MSVC)
    target_link_libraries(${lib_name} INTERFACE Microsoft::STL)
  endif()
  target_include_directories(
    ${lib_name} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                          $<INSTALL_INTERFACE:include/cyy/naive_lib>)

  if(submodule_name STREQUAL "python_binding")
    continue()
  endif()
  set_target_properties(${lib_name} PROPERTIES EXPORT_NAME ${submodule_name})
  list(APPEND submodule_libs ${lib_name})

  install(
    DIRECTORY ${submodule_name}/
    DESTINATION include/cyy/naive_lib/${submodule_name}
    FILES_MATCHING
    PATTERN "*.hpp")
endforeach()

install(
  TARGETS ${submodule_libs}
  EXPORT CyyNaiveLibConfig
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib COMPONENT library
  INCLUDES
  DESTINATION include)

install(
  EXPORT CyyNaiveLibConfig
  DESTINATION lib/cmake/${PROJECT_NAME}
  NAMESPACE CyyNaiveLib::)
