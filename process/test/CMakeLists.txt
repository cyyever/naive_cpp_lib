include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/test.cmake)

find_package(doctest REQUIRED)

set(test_progs process_test monitor_test thread_test)

foreach(test_prog ${test_progs})
  add_executable(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  target_link_libraries(${test_prog} PRIVATE MyCXXLib::process)
  target_link_libraries(${test_prog} PRIVATE MyCXXLib::log)
  target_link_libraries(${test_prog} PRIVATE doctest::doctest)
  target_compile_definitions(${test_prog}
                             PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
  add_test_with_runtime_analysis(TARGET ${test_prog})
endforeach()
