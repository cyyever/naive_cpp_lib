file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(my_cxx_lib_torch ${SOURCE})
add_library(MyCXXLib::torch ALIAS my_cxx_lib_torch)
target_link_libraries(my_cxx_lib_torch PRIVATE MyCXXLib::log)
target_link_libraries(my_cxx_lib_torch PRIVATE MyCXXLib::util)

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
  list(PREPEND CMAKE_PREFIX_PATH ${_path})
endif()

find_package(Torch REQUIRED)

target_link_libraries(my_cxx_lib_torch PUBLIC ${TORCH_LIBRARIES})

if(BUILD_TEST)
  add_subdirectory(test)
endif()
