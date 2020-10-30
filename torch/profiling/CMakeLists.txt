include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/profiling.cmake)

file(GLOB profiling_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp)
foreach(profiling_source IN LISTS profiling_sources)
  get_filename_component(profiling_prog ${profiling_source} NAME_WE)
  add_executable(${profiling_prog}
                 ${CMAKE_CURRENT_LIST_DIR}/${profiling_prog}.cpp)
  target_link_libraries(${profiling_prog} PRIVATE CyyNaiveLib::torch CyyNaiveLib::log
                                                  CyyNaiveLib::util)
  add_profiling(TARGET ${profiling_prog} WITH_GPU_profiling OFF LTRACE OFF)
endforeach()
