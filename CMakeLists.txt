cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_MODULE_STD 1)
if(WIN32)
  set(CMAKE_CXX_STANDARD 23)
else()
  set(CMAKE_CXX_STANDARD 26)
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_C_COMPILER clang)
  set(CMAKE_C_FLAGS -stdlib=libc)
  set(CMAKE_CXX_FLAGS -stdlib=libc++)
endif()

project(CyyNaiveLib LANGUAGES CXX)

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
