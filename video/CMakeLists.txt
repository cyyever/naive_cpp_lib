file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB AUDIO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*audio*.cpp)
list(REMOVE_ITEM SOURCE ${IMPL_SOURCE} ${AUDIO_SOURCE})
list(APPEND AUDIO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/ffmpeg_base.cpp)

add_library(cyy_naive_lib_video ${SOURCE})
add_library(CyyNaiveLib::video ALIAS cyy_naive_lib_video)
add_library(cyy_naive_lib_audio ${AUDIO_SOURCE})
add_library(CyyNaiveLib::audio ALIAS cyy_naive_lib_audio)

# dependency
target_link_libraries(cyy_naive_lib_video PRIVATE CyyNaiveLib::log)
target_link_libraries(cyy_naive_lib_audio PRIVATE CyyNaiveLib::util)
target_link_libraries(cyy_naive_lib_audio PRIVATE CyyNaiveLib::data_structure)

find_package(PkgConfig REQUIRED)
pkg_search_module(libavcodec REQUIRED IMPORTED_TARGET libavcodec)
pkg_search_module(libswscale REQUIRED IMPORTED_TARGET libswscale)
pkg_search_module(libavformat REQUIRED IMPORTED_TARGET libavformat)
pkg_search_module(libavutil REQUIRED IMPORTED_TARGET libavutil)
pkg_search_module(libavdevice REQUIRED IMPORTED_TARGET libavdevice)
target_link_libraries(
  cyy_naive_lib_video
  PUBLIC PkgConfig::libavcodec PkgConfig::libavutil PkgConfig::libavformat
         PkgConfig::libswscale PkgConfig::libavdevice)
target_link_libraries(
  cyy_naive_lib_audio PUBLIC PkgConfig::libavcodec PkgConfig::libavutil
                             PkgConfig::libavformat PkgConfig::libavdevice)

find_package(OpenCV REQUIRED COMPONENTS core)

target_include_directories(cyy_naive_lib_video PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(cyy_naive_lib_video PRIVATE ${OpenCV_LIBRARIES})
target_link_libraries(cyy_naive_lib_video PUBLIC opencv_core)

if(BUILD_PYTHON_BINDING)
  unset(Python3_FOUND)
  find_package(pybind11 CONFIG REQUIRED)
  file(GLOB PYTHON_BINDING_SOURCE
       ${CMAKE_CURRENT_SOURCE_DIR}/python_binding/*.cpp)

  if(TARGET cyy_naive_cpp_extension)
    target_sources(cyy_naive_cpp_extension PUBLIC ${PYTHON_BINDING_SOURCE})
  else()
    pybind11_add_module(cyy_naive_cpp_extension ${PYTHON_BINDING_SOURCE})
  endif()
  target_link_directories(cyy_naive_cpp_extension PUBLIC ${pytorch_path}/lib)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC cyy_naive_lib_video
                                                       ${PYTHON_LIBRARY})
endif()

add_subdirectory(test)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()
add_subdirectory(example)
