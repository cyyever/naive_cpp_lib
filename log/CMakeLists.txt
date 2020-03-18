file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(my_cxx_lib_log ${SOURCE})

find_package(spdlog REQUIRED)
target_link_libraries(my_cxx_lib_log PUBLIC spdlog::spdlog)

find_package(Threads REQUIRED)
target_link_libraries(my_cxx_lib_log PRIVATE Threads::Threads)
add_library(MyCXXLib::log ALIAS my_cxx_lib_log)
if(BUILD_TEST)
  add_subdirectory(test)
endif()
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
