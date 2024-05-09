cmake_minimum_required(VERSION 3.10)

project(media_pusher CXX)

find_package(Threads REQUIRED)

set(PATH_3RDPARTY ${CMAKE_SOURCE_DIR}/3rdparty/)
include_directories(${PATH_3RDPARTY})

# boost
add_definitions(-DBOOST_ASIO_STANDALONE)

# ffmpeg
set(LIST_FFMPEG "")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(PATH_FFMPEG ${PATH_3RDPARTY}/ffmpeg-4.1-win64-dev)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(PATH_FFMPEG ${PATH_3RDPARTY}/ffmpeg-4.4.2-ubuntu/)

    # x264
    set(PATH_X264 ${PATH_3RDPARTY}/x264-ubuntu/)
    include_directories(${PATH_X264}/include)
    link_directories(${PATH_X264}/lib)
    list(APPEND LIST_FFMPEG x264)
endif()
include_directories(${PATH_FFMPEG}/include)
link_directories(${PATH_FFMPEG}/lib)
set(LIST_FFMPEG avcodec avdevice avfilter avformat avutil swresample swscale)


set(OPENSSL_PATH_LIB ${PATH_3RDPARTY}/openssl/)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    include_directories(${OPENSSL_PATH_LIB}/include/)
    link_directories(${OPENSSL_PATH_LIB}/lib/)
    set(LIST_OPENSSL_LIB libcrypto libssl)
endif()

#dxgi d3d11 d3d9 -ldxva2
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(DXGI dxgi)
    set(D3D11 d3d11)
    set(D3D9 d3d9)
    set(DXVA2 dxva2)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    #set(WS2_32 ${PATH_3RDPARTY}/opencv-3.4.1-ubuntu/share/OpenCV)
endif()

#Iphlpapi
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(IPHLPAPI Iphlpapi)
endif()

include_directories(${CMAKE_SOURCE_DIR})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# aux_source_directory(.              SRC_LIB)
#aux_source_directory(./onvif   SRC_ONVIF)
#list(APPEND SRC_LIB ${SRC_ONVIF})

FILE (GLOB ALL_SOURCES "*.cpp" "*.c" "./capture/*.cpp"  
 "./3rdparty/onvif/*.cpp" "./3rdparty/onvif/*.nsmap" 
"./3rdparty/net/*.cpp" "./3rdparty/xop/*.cpp")
FILE (GLOB ALL_INCLUDES "*.hpp" "*.h" "./capture/*.h"
"./3rdparty/net/*.h" "./3rdparty/xop/*.h" "./3rdparty/md5/*.hpp" "./3rdparty/onvif/*.h")


SET (ALL_SRCS 
	 ${ALL_SOURCES}
	 ${ALL_INCLUDES}
)


 add_definitions(-DNDEBUG)
 add_definitions(-DWITH_OPENSSL)
add_definitions(-DMediaPusherSingle_EXPORTS)
add_definitions(-DNOMINMAX)
add_definitions(-D__WINDOWS_WASAPI_)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#add_library(media_pusher_single SHARED ${ALL_SRCS})
add_library(media_pusher_single ${ALL_SRCS})
#target_compile_definitions(media_pusher_single PRIVATE _ITERATOR_DEBUG_LEVEL=0)
target_link_libraries(media_pusher_single  ${LIST_FFMPEG} ${LIST_OPENSSL_LIB} ${CMAKE_THREAD_LIBS_INIT} ${IPHLPAPI} ${DXGI} ${D3D11} ${D3D9} ${DXVA2})