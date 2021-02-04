set(fuzzing_progs reader_fuzzing writer_fuzzing)

foreach(fuzzing_prog ${fuzzing_progs})
  add_executable(${fuzzing_prog} ${CMAKE_CURRENT_LIST_DIR}/${fuzzing_prog}.cpp)
  target_link_libraries(${fuzzing_prog} PRIVATE CyyNaiveLib::video)
  target_link_libraries(${fuzzing_prog} PRIVATE CyyNaiveLib::cv)
  target_compile_definitions(${fuzzing_prog}
                             PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
  set_target_properties(${fuzzing_prog} PROPERTIES ENVIRONMENT TEST_NO_CUDA=1)
  add_fuzzing(TARGET ${fuzzing_prog})
endforeach()
