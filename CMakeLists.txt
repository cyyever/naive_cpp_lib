cmake_minimum_required(VERSION 4.1.0)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_MODULE_STD 1)
set(CMAKE_CXX_STANDARD 23)

project(CyyNaiveLib LANGUAGES CXX C)

set(submodule_names log util)
option(BUILD_OBJECT_LIB "Build CMake object libraries" OFF)
option(BUILD_ALL "Build all modules" OFF)
option(BUILD_CV "Build opencv module" OFF)
option(BUILD_VIDEO "Build video modules" OFF)
option(BUILD_CV_PYTHON_BINDING "Build python bindings for cv" OFF)
option(BUILD_VIDEO_PYTHON_BINDING "Build python bindings for video" OFF)

if(BUILD_ALL)
  set(BUILD_CV ON)
  set(BUILD_VIDEO ON)
  set(BUILD_CV_PYTHON_BINDING ON)
  set(BUILD_VIDEO_PYTHON_BINDING ON)
endif()
if(BUILD_VIDEO)
  set(BUILD_CV ON)
endif()

if(BUILD_CV)
  list(APPEND submodule_names cv)
endif()

if(BUILD_VIDEO)
  list(APPEND submodule_names video)
endif()

if(BUILD_VIDEO_PYTHON_BINDING OR BUILD_CV_PYTHON_BINDING)
  list(APPEND submodule_names python_binding)
endif()

option(BUILD_FUZZING "Build fuzzing" OFF)

include(cmake/all.cmake)

set(submodule_libs)

foreach(submodule_name IN LISTS submodule_names)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${submodule_name})
  set(lib_name cyy_naive_lib_${submodule_name})
  if(submodule_name STREQUAL "python_binding")
    set(lib_name cyy_naive_cpp_extension)
    continue()
  endif()
  set_target_properties(${lib_name} PROPERTIES EXPORT_NAME ${submodule_name})
  list(APPEND submodule_libs ${lib_name})

  if(NOT BUILD_OBJECT_LIB)
    install(
      TARGETS ${lib_name}
      EXPORT CyyNaiveLibConfig
      FILE_SET public_headers
      DESTINATION include/cyy/naive_lib/)
  endif()
endforeach()

if(NOT BUILD_OBJECT_LIB)
  install(
    EXPORT CyyNaiveLibConfig
    DESTINATION lib/cmake/${PROJECT_NAME}
    NAMESPACE CyyNaiveLib::)
endif()
