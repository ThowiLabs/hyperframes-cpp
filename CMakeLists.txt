cmake_minimum_required(VERSION 3.16)
project(hyperframes_cpp_native LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(HFC_HTML_LAYOUT "Build the litehtml HTML/CSS video renderer" ON)
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/font_embedded.hpp")
  message(FATAL_ERROR "Run python3 scripts/embed_fonts.py before configuring")
endif()
add_executable(hyperframes-cpp-native main.cpp)
if (HFC_HTML_LAYOUT)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/_sources/litehtml/CMakeLists.txt")
    message(FATAL_ERROR "litehtml source missing: run scripts/fetch-html-deps.sh")
  endif()
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "Static litehtml" FORCE)
  set(LITEHTML_ENABLE_LINT OFF CACHE BOOL "" FORCE)
  set(LITEHTML_BUILD_TESTING OFF CACHE BOOL "" FORCE)
  add_subdirectory(third_party/_sources/litehtml ${CMAKE_BINARY_DIR}/thirdparty/litehtml EXCLUDE_FROM_ALL)
  target_link_libraries(hyperframes-cpp-native PRIVATE litehtml)
  target_compile_definitions(hyperframes-cpp-native PRIVATE HFC_WITH_LITEHTML)
  add_executable(hyperframes-html-preview html_layout_preview.cpp)
  target_link_libraries(hyperframes-html-preview PRIVATE litehtml)
endif()
option(HFC_NATIVE_MP4 "Build native H.264/AAC MP4 encoder" OFF)
if(HFC_NATIVE_MP4)
  foreach(dep minih264 minimp4 vo-aacenc)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/_sources/${dep}")
      message(FATAL_ERROR "${dep} missing, run scripts/fetch-codecs.sh")
    endif()
  endforeach()
  set(CODEC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/_sources")
  file(GLOB AAC_SRC "${CODEC_ROOT}/vo-aacenc/aacenc/src/*.c")
  add_library(hfc-native-codecs STATIC
    native_codecs/h264_impl.c
    native_codecs/mp4_impl.c
    "${CODEC_ROOT}/vo-aacenc/common/cmnMemory.c"
    "${CODEC_ROOT}/vo-aacenc/aacenc/basic_op/basicop2.c"
    "${CODEC_ROOT}/vo-aacenc/aacenc/basic_op/oper_32b.c"
    ${AAC_SRC})
  target_compile_definitions(hfc-native-codecs PRIVATE H264E_MAX_THREADS=0 __unused=)
  target_include_directories(hfc-native-codecs PUBLIC
    "${CODEC_ROOT}/minih264"
    "${CODEC_ROOT}/minimp4"
    "${CODEC_ROOT}/vo-aacenc"
    "${CODEC_ROOT}/vo-aacenc/aacenc/inc"
    "${CODEC_ROOT}/vo-aacenc/aacenc/basic_op"
    "${CODEC_ROOT}/vo-aacenc/common/include")
  target_compile_definitions(hyperframes-cpp-native PRIVATE HFC_WITH_NATIVE_MP4)
  target_link_libraries(hyperframes-cpp-native PRIVATE hfc-native-codecs)
  if(NOT WIN32)
    target_link_libraries(hyperframes-cpp-native PRIVATE m)
  endif()
endif()

option(HFC_BROWSER_CAPTURE "Enable installed-browser CDP capture mode" ON)
if(HFC_BROWSER_CAPTURE)
  target_compile_definitions(hyperframes-cpp-native PRIVATE HFC_WITH_BROWSER_CAPTURE)
  if(WIN32)
    target_link_libraries(hyperframes-cpp-native PRIVATE ws2_32)
  endif()
endif()

# Keep all C/C++ runtime libraries within the executable; external browser
# execution is intentionally the only required runtime program for browser mode.
if(MINGW)
  target_link_options(hyperframes-cpp-native PRIVATE -static -static-libgcc -static-libstdc++)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(hyperframes-cpp-native PRIVATE -static -static-libgcc -static-libstdc++)
endif()
