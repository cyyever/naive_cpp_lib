file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(my_cxx_lib_util ${SOURCE})

find_package(Threads REQUIRED)

target_link_libraries(my_cxx_lib_util PRIVATE MyCXXLib::log)
target_link_libraries(my_cxx_lib_util PUBLIC Threads::Threads)

add_library(MyCXXLib::util ALIAS my_cxx_lib_util)
if(BUILD_TEST)
  add_subdirectory(test)
endif()
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
