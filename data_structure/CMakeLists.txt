# file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
find_package(Threads REQUIRED)

add_library(my_cxx_lib_data_structure INTERFACE)
target_include_directories(my_cxx_lib_data_structure INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(my_cxx_lib_data_structure INTERFACE Threads::Threads)

add_library(MyCXXLib::data_structure ALIAS my_cxx_lib_data_structure)
add_subdirectory(test)
