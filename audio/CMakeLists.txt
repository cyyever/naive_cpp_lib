file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(APPEND SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../video/src/ffmpeg_base.cpp)

add_library(cyy_naive_lib_audio ${AUDIO_SOURCE})
add_library(CyyNaiveLib::audio ALIAS cyy_naive_lib_audio)

# dependency
target_link_libraries(cyy_naive_lib_audio PRIVATE CyyNaiveLib::util)
target_link_libraries(cyy_naive_lib_audio PRIVATE CyyNaiveLib::data_structure)

find_package(PkgConfig REQUIRED)
pkg_search_module(libavcodec REQUIRED IMPORTED_TARGET libavcodec)
pkg_search_module(libavformat REQUIRED IMPORTED_TARGET libavformat)
pkg_search_module(libavutil REQUIRED IMPORTED_TARGET libavutil)
pkg_search_module(libavdevice REQUIRED IMPORTED_TARGET libavdevice)
target_link_libraries(
  PUBLIC PkgConfig::libavcodec PkgConfig::libavutil PkgConfig::libavformat
         PkgConfig::libswscale PkgConfig::libavdevice)
target_link_libraries(
  cyy_naive_lib_audio PUBLIC PkgConfig::libavcodec PkgConfig::libavutil
                             PkgConfig::libavformat PkgConfig::libavdevice)

add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
# add_subdirectory(example)
