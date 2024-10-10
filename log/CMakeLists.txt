file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

if(BUILD_OBJECT_LIB)
  add_library(cyy_naive_lib_log OBJECT ${SOURCE})
else()
  add_library(cyy_naive_lib_log ${SOURCE})
  find_package(spdlog REQUIRED)
endif()
target_sources(
  cyy_naive_lib_log
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/..
         FILES
         ${headers})

target_link_libraries(cyy_naive_lib_log PUBLIC spdlog::spdlog_header_only)
set_target_properties(cyy_naive_lib_log PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(CyyNaiveLib::log ALIAS cyy_naive_lib_log)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
