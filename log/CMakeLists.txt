file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(my_cxx_lib_log ${SOURCE})

find_package(spdlog REQUIRED)
target_link_libraries(my_cxx_lib_log PUBLIC spdlog::spdlog_header_only)

add_library(CyyNaiveLib::log ALIAS my_cxx_lib_log)
add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
