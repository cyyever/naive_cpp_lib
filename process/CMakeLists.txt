file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(cyy_naive_lib_process ${SOURCE})
add_library(CyyNaiveLib::process ALIAS cyy_naive_lib_process)

target_link_libraries(cyy_naive_lib_process PRIVATE CyyNaiveLib::log)

target_link_libraries(cyy_naive_lib_process PRIVATE CyyNaiveLib::util)

find_package(Threads REQUIRED)
target_link_libraries(cyy_naive_lib_process PUBLIC Threads::Threads)

# test
add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
