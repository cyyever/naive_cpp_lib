file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(my_cxx_lib_data_structure ${SOURCE})

add_library(MyCXXLib::data_structure ALIAS my_cxx_lib_data_structure)
add_subdirectory(test)
