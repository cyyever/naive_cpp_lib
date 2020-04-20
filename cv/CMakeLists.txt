file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(my_cxx_lib_cv ${SOURCE})
add_library(MyCXXLib::cv ALIAS my_cxx_lib_cv)
target_link_libraries(my_cxx_lib_cv PRIVATE MyCXXLib::log)
target_link_libraries(my_cxx_lib_cv PRIVATE MyCXXLib::util)
target_link_libraries(my_cxx_lib_cv PRIVATE MyCXXLib::hardware)

find_package(
  OpenCV REQUIRED
  COMPONENTS core imgproc imgcodecs
  OPTIONAL_COMPONENTS cudaarithm cudaimgproc)

target_include_directories(my_cxx_lib_cv PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(my_cxx_lib_cv PRIVATE ${OpenCV_LIBRARIES})
target_link_libraries(my_cxx_lib_cv PUBLIC opencv_core)

if("opencv_cudaarithm" IN_LIST OpenCV_LIB_COMPONENTS)
  target_compile_definitions(my_cxx_lib_cv PUBLIC HAVE_GPU_MAT)
  find_package(CUDAToolkit REQUIRED)
  target_link_libraries(my_cxx_lib_cv PRIVATE CUDA::cudart CUDA::cudart_static)
  find_package(CUDABuddyAllocator REQUIRED)
  find_package(spdlog REQUIRED)
  find_package(Threads REQUIRED)
  target_link_libraries(my_cxx_lib_cv PRIVATE CUDABuddyAllocator)
  target_link_libraries(my_cxx_lib_cv PRIVATE Threads::Threads)
endif()

if(BUILD_TEST)
  add_subdirectory(test)
endif()
# if(BUILD_FUZZING) add_subdirectory(fuzz_test) endif()