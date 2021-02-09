file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(cyy_naive_lib_torch ${SOURCE})
add_library(CyyNaiveLib::torch ALIAS cyy_naive_lib_torch)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::log)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::hardware)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::util)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::data_structure)

find_package(Python3 REQUIRED)
if(Python3_FOUND)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import torch ; print(torch.__path__[0])"
    RESULT_VARIABLE _exitcode
    OUTPUT_VARIABLE _path)
  if(NOT ${_exitcode} EQUAL 0)
    message(FATAL_ERROR "failed to found pytorch path")
  endif()
  string(STRIP ${_path} pytorch_path)
  list(PREPEND CMAKE_PREFIX_PATH ${pytorch_path}/share/cmake)
endif()

find_package(Torch REQUIRED)
target_link_libraries(cyy_naive_lib_torch PUBLIC ${TORCH_LIBRARIES})
find_package(Threads REQUIRED)
target_link_libraries(cyy_naive_lib_torch PUBLIC Threads::Threads)

option(BUILD_TORCH_PYTHON_BINDING "Build torch python bindings" ON)
if(BUILD_TORCH_PYTHON_BINDING)
  unset(Python3_FOUND)
  find_package(pybind11 CONFIG REQUIRED)
  file(GLOB PYTHON_BINDING_SOURCE
       ${CMAKE_CURRENT_SOURCE_DIR}/python_binding/*.cpp)
  if(TARGET cyy_naive_cpp_extension)
    target_sources(cyy_naive_cpp_extension PUBLIC ${PYTHON_BINDING_SOURCE})
  else()
    pybind11_add_module(cyy_naive_cpp_extension ${PYTHON_BINDING_SOURCE})
  endif()
  target_link_directories(cyy_naive_cpp_extension PUBLIC ${pytorch_path}/lib)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC torch_python)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC cyy_naive_lib_torch
                                                      ${PYTHON_LIBRARY})
endif()

add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
add_subdirectory(profiling)
