file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(my_cxx_lib_hardware ${SOURCE})

find_package(CUDAToolkit)

if(CUDAToolkit_FOUND)
  target_compile_definitions(my_cxx_lib_hardware PUBLIC HAVE_CUDA)
  target_link_libraries(my_cxx_lib_hardware PUBLIC CUDA::cudart
                                                   CUDA::cudart_static)
endif()

add_library(MyCXXLib::hardware ALIAS my_cxx_lib_hardware)
target_link_libraries(my_cxx_lib_hardware PRIVATE MyCXXLib::util)
target_link_libraries(my_cxx_lib_hardware PRIVATE MyCXXLib::log)
if(BUILD_TEST)
  add_subdirectory(test)
endif()
