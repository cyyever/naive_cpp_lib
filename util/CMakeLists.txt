file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(cyy_naive_lib_util ${SOURCE})

find_package(Threads REQUIRED)

target_link_libraries(cyy_naive_lib_util PRIVATE CyyNaiveLib::log)
target_link_libraries(cyy_naive_lib_util PRIVATE Threads::Threads)

add_library(CyyNaiveLib::util ALIAS cyy_naive_lib_util)
add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
