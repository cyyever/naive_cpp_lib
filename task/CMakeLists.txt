file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(my_cxx_lib_task ${SOURCE})

# dependency find_package(DeepirUtil REQUIRED) find_package(DeepirHardware
# REQUIRED)
target_link_libraries(my_cxx_lib_task PRIVATE my_cxx_lib_util
                                              my_cxx_lib_hardware)

find_package(Threads REQUIRED)
target_link_libraries(my_cxx_lib_task PUBLIC Threads::Threads)

# add_subdirectory(test)
