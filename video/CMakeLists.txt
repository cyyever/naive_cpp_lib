file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(cyy_naive_lib_video ${SOURCE})
add_library(CyyNaiveLib::video ALIAS cyy_naive_lib_video)

# dependency
target_link_libraries(cyy_naive_lib_video PRIVATE CyyNaiveLib::log)
target_link_libraries(cyy_naive_lib_video PRIVATE CyyNaiveLib::util)
# cv is PUBLIC because cv::Mat appears in video's public headers
find_package(CyyAlgorithmLib REQUIRED)
target_link_libraries(cyy_naive_lib_video PRIVATE CyyAlgorithmLib)

find_package(PkgConfig REQUIRED)
pkg_search_module(libavcodec REQUIRED IMPORTED_TARGET libavcodec)
pkg_search_module(libswscale REQUIRED IMPORTED_TARGET libswscale)
pkg_search_module(libavformat REQUIRED IMPORTED_TARGET libavformat)
pkg_search_module(libavutil REQUIRED IMPORTED_TARGET libavutil)
pkg_search_module(libavdevice REQUIRED IMPORTED_TARGET libavdevice)
target_link_libraries(
  cyy_naive_lib_video
  PRIVATE PkgConfig::libavcodec PkgConfig::libavutil PkgConfig::libavformat
         PkgConfig::libswscale PkgConfig::libavdevice)

target_link_libraries(cyy_naive_lib_video PUBLIC CyyNaiveLib::cv)

add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
# add_subdirectory(example)
