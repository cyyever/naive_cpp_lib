file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(cyy_naive_lib_task ${SOURCE})
add_library(CyyNaiveLib::task ALIAS cyy_naive_lib_task)

find_package(CyyAlgorithmLib REQUIRED)
target_link_libraries(cyy_naive_lib_task PRIVATE cyy_naive_lib_hardware
                                                 cyy_naive_lib_log)
target_link_libraries(cyy_naive_lib_task PRIVATE CyyAlgorithmLib)

target_link_libraries(cyy_naive_lib_task PUBLIC CyyNaiveLib::util)
find_package(Threads REQUIRED)
target_link_libraries(cyy_naive_lib_task PUBLIC Threads::Threads)

add_subdirectory(test)
