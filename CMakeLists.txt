cmake_minimum_required(VERSION 3.19)

project(CyyNaiveLib LANGUAGES CXX)

set(submodule_names log util hardware data_structure task)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # list(APPEND submodule_names process)
endif()
option(BUILD_ALL "Build all modules" OFF)
option(BUILD_TORCH "Build torch module" OFF)
option(BUILD_CV "Build opencv module" OFF)
option(BUILD_VIDEO "Build video modules" ON)
option(BUILD_TORCH_PYTHON_BINDING "Build python bindings for torch" OFF)
option(BUILD_VIDEO_PYTHON_BINDING "Build python bindings for video" OFF)

if(BUILD_ALL)
  set(BUILD_TORCH ON)
  set(BUILD_CV ON)
  set(BUILD_VIDEO ON)
  set(BUILD_TORCH_PYTHON_BINDING ON)
  set(BUILD_VIDEO_PYTHON_BINDING ON)
endif()
if(BUILD_VIDEO)
  set(BUILD_CV ON)
endif()

set(HAS_CUDA False)
if(BUILD_TORCH)
  include(CheckLanguage)
  check_language(CUDA)
  if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    add_compile_definitions(HAS_CUDA)
    set(HAS_CUDA True)
  else()
    message(WARING "No CUDA support")
  endif()
  list(APPEND submodule_names torch)
endif()
if(BUILD_CV)
  list(APPEND submodule_names cv)
endif()

if(BUILD_VIDEO)
  list(APPEND submodule_names video)
endif()

if(BUILD_TORCH_PYTHON_BINDING OR BUILD_VIDEO_PYTHON_BINDING)
  list(APPEND submodule_names python_binding)
endif()

option(WITH_ASAN "Build with address sanitizer" OFF)

option(BUILD_FUZZING "Build fuzzing" OFF)

include(cmake/all.cmake)

set(submodule_libs)

foreach(submodule_name IN LISTS submodule_names)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${submodule_name})
  set(lib_name cyy_naive_lib_${submodule_name})
  if(submodule_name STREQUAL "python_binding")
    set(lib_name cyy_naive_cpp_extension)
  endif()
  if(WITH_ASAN)
    find_package(GoogleSanitizer REQUIRED)
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
