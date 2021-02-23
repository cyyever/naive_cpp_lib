find_package(doctest REQUIRED)

set(TEST_AUDIO_DIR ${CMAKE_CURRENT_LIST_DIR}/test_audio)
foreach(test_prog audio_reader_test)
  add_executable(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  target_link_libraries(${test_prog} PRIVATE CyyNaiveLib::audio)
  target_link_libraries(${test_prog} PRIVATE doctest::doctest)
  target_compile_definitions(${test_prog}
                             PRIVATE IN_AUDIO=${TEST_AUDIO_DIR}/1.pcm_mulaw)
  target_compile_definitions(${test_prog}
                             PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
  add_test_with_runtime_analysis(TARGET ${test_prog})
endforeach()
