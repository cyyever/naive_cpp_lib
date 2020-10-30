# file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
find_package(Threads REQUIRED)

add_library(cyy_naive_lib_data_structure INTERFACE)
target_include_directories(cyy_naive_lib_data_structure INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cyy_naive_lib_data_structure INTERFACE Threads::Threads)

add_library(CyyNaiveLib::data_structure ALIAS cyy_naive_lib_data_structure)
add_subdirectory(test)
