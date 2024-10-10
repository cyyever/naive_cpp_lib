file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

if(BUILD_OBJECT_LIB)
  add_library(cyy_naive_lib_util OBJECT ${SOURCE})
else()
  add_library(cyy_naive_lib_util ${SOURCE})
endif()

find_package(Threads REQUIRED)

target_link_libraries(cyy_naive_lib_util PRIVATE cyy_naive_lib_log)
target_link_libraries(cyy_naive_lib_util PRIVATE Threads::Threads)
target_sources(
  cyy_naive_lib_util
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/..
         FILES
         ${headers})

add_library(CyyNaiveLib::util ALIAS cyy_naive_lib_util)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
