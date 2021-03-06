find_package(doctest REQUIRED)
file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp)
foreach(test_source IN LISTS test_sources)
  get_filename_component(test_prog ${test_source} NAME_WE)
  add_executable(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  target_link_libraries(${test_prog} PRIVATE CyyNaiveLib::hardware)
  target_link_libraries(${test_prog} PRIVATE doctest::doctest)
  target_compile_definitions(${test_prog}
                             PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

  if(test_prog STREQUAL cuda_test)
    add_test_with_runtime_analysis(TARGET ${test_prog} ASAN FALSE)
  else()
    add_test_with_runtime_analysis(TARGET ${test_prog})
  endif()
endforeach()
