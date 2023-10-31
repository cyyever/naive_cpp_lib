file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if(BUILD_OBJECT_LIB)
  add_library(cyy_naive_lib_log OBJECT ${SOURCE})
else()
  add_library(cyy_naive_lib_log ${SOURCE})
endif()

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
target_link_libraries(cyy_naive_lib_log PUBLIC spdlog::spdlog_header_only)
set_target_properties(cyy_naive_lib_log PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(CyyNaiveLib::log ALIAS cyy_naive_lib_log)
add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
