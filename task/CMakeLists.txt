file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(my_cxx_lib_task ${SOURCE})

target_link_libraries(my_cxx_lib_task PRIVATE my_cxx_lib_hardware
                                              my_cxx_lib_data_structure)

target_link_libraries(my_cxx_lib_task PUBLIC my_cxx_lib_util)
find_package(Threads REQUIRED)
target_link_libraries(my_cxx_lib_task PUBLIC Threads::Threads)

add_subdirectory(test)
