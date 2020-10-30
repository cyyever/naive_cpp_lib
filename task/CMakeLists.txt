file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(cyy_naive_lib_task ${SOURCE})

target_link_libraries(cyy_naive_lib_task PRIVATE cyy_naive_lib_hardware
                                              cyy_naive_lib_log
                                              cyy_naive_lib_data_structure)

target_link_libraries(cyy_naive_lib_task PUBLIC cyy_naive_lib_util)
find_package(Threads REQUIRED)
target_link_libraries(cyy_naive_lib_task PUBLIC Threads::Threads)

add_subdirectory(test)
