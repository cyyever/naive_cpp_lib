file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
if(HAS_CUDA)
  file(GLOB CUDA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cu)
  list(APPEND SOURCE ${CUDA_SOURCE})
endif()

find_package(Python3 REQUIRED)
if(Python3_FOUND)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import torch ; print(torch.__path__[0])"
    RESULT_VARIABLE _exitcode
    OUTPUT_VARIABLE _path)
  if(NOT ${_exitcode} EQUAL 0)
    message(FATAL_ERROR "failed to found pytorch path")
  endif()
  string(STRIP ${_path} _path)
  set(pytorch_path
      ${_path}
      CACHE STRING "pytorch path" FORCE)
  list(PREPEND CMAKE_PREFIX_PATH ${pytorch_path}/share/cmake)
endif()

find_package(Torch REQUIRED)

add_library(cyy_naive_lib_torch ${SOURCE})
add_library(CyyNaiveLib::torch ALIAS cyy_naive_lib_torch)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::log)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::util)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::hardware)
target_link_libraries(cyy_naive_lib_torch PRIVATE CyyNaiveLib::data_structure)
target_link_directories(cyy_naive_lib_torch PUBLIC ${pytorch_path}/lib)
target_link_libraries(cyy_naive_lib_torch PUBLIC ${TORCH_LIBRARIES} torch_cuda)
target_compile_definitions(
  cyy_naive_lib_torch PRIVATE __CUDA_NO_HALF_OPERATORS__ CUDA_HAS_FP16=1
                              __CUDA_NO_HALF_CONVERSIONS__)

add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
# add_subdirectory(profiling)
