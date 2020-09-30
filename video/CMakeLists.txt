file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB AUDIO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*audio*.cpp)
list(REMOVE_ITEM SOURCE ${IMPL_SOURCE} ${AUDIO_SOURCE})
list(APPEND AUDIO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/ffmpeg_base.cpp)

add_library(my_cxx_lib_video ${SOURCE})
add_library(MyCXXLib::video ALIAS my_cxx_lib_video)
add_library(my_cxx_lib_audio ${AUDIO_SOURCE})
add_library(MyCXXLib::audio ALIAS my_cxx_lib_audio)

# dependency
target_link_libraries(my_cxx_lib_video PRIVATE MyCXXLib::log)
target_link_libraries(my_cxx_lib_audio PRIVATE MyCXXLib::util)
target_link_libraries(my_cxx_lib_audio PRIVATE MyCXXLib::data_structure)

find_package(PkgConfig REQUIRED)
pkg_search_module(libavcodec REQUIRED IMPORTED_TARGET libavcodec)
pkg_search_module(libswscale REQUIRED IMPORTED_TARGET libswscale)
pkg_search_module(libavformat REQUIRED IMPORTED_TARGET libavformat)
pkg_search_module(libavutil REQUIRED IMPORTED_TARGET libavutil)
pkg_search_module(libavdevice REQUIRED IMPORTED_TARGET libavdevice)
target_link_libraries(
  my_cxx_lib_video
  PUBLIC PkgConfig::libavcodec PkgConfig::libavutil PkgConfig::libavformat
         PkgConfig::libswscale PkgConfig::libavdevice)
target_link_libraries(
  my_cxx_lib_audio PUBLIC PkgConfig::libavcodec PkgConfig::libavutil
                          PkgConfig::libavformat PkgConfig::libavdevice)

find_package(OpenCV REQUIRED COMPONENTS core)

target_include_directories(my_cxx_lib_video PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(my_cxx_lib_video PRIVATE ${OpenCV_LIBRARIES})
target_link_libraries(my_cxx_lib_video PUBLIC opencv_core)

add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
