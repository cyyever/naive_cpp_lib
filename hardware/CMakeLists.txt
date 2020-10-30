file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(cyy_naive_lib_hardware ${SOURCE})

find_package(CUDAToolkit)

if(CUDAToolkit_FOUND)
  target_compile_definitions(cyy_naive_lib_hardware PUBLIC HAVE_CUDA)
  target_link_libraries(cyy_naive_lib_hardware PUBLIC CUDA::cudart
                                                   CUDA::cudart_static)
endif()

add_library(CyyNaiveLib::hardware ALIAS cyy_naive_lib_hardware)
target_link_libraries(cyy_naive_lib_hardware PRIVATE CyyNaiveLib::util)
target_link_libraries(cyy_naive_lib_hardware PRIVATE CyyNaiveLib::log)
add_subdirectory(test)
