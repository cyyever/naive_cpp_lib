file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(my_cxx_lib_process ${SOURCE})
add_library(MyCXXLib::process ALIAS my_cxx_lib_process)

target_link_libraries(my_cxx_lib_process PRIVATE MyCXXLib::log)

target_link_libraries(my_cxx_lib_process PRIVATE MyCXXLib::util)

find_package(Threads REQUIRED)
target_link_libraries(my_cxx_lib_process PUBLIC Threads::Threads)

# test
add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
