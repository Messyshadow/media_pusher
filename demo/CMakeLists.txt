cmake_minimum_required(VERSION 3.10)
project(demo_media_pusher_single CXX)
project(demo_media_finder_onvif CXX)
project(demo_media_finder_monitor CXX)

find_package(Threads REQUIRED)

#media_pusher_single
set(PATH_MEDIA_PUSHER ${CMAKE_SOURCE_DIR}/../)
include_directories(${PATH_MEDIA_PUSHER$})
link_directories(${PATH_MEDIA_PUSHER}/lib/)
set(LIB_MEDIA_PUSHER media_pusher_single)

# ffmpeg
set(LIST_FFMPEG "")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(PATH_FFMPEG ${PATH_MEDIA_PUSHER}/3rdparty/ffmpeg-4.1-win64-dev)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(PATH_FFMPEG ${PATH_MEDIA_PUSHER}/3rdparty/ffmpeg-4.4.2-ubuntu/)
endif()
include_directories(${PATH_FFMPEG}/include)
link_directories(${PATH_FFMPEG}/lib)
set(LIST_FFMPEG avcodec avdevice avfilter avformat avutil swresample swscale)

# openssl
set(OPENSSL_PATH_LIB ${PATH_MEDIA_PUSHER}/3rdparty/openssl/)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    include_directories(${OPENSSL_PATH_LIB}/include/)
    link_directories(${OPENSSL_PATH_LIB}/lib/)
    set(LIST_OPENSSL_LIB libcrypto libssl)
endif()

#Iphlpapi
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(IPHLPAPI Iphlpapi)
endif()

FILE (GLOB ALL_SOURCES "*.cpp")
FILE (GLOB ALL_INCLUDES "*.h")

SET (ALL_SRCS 
	 ${ALL_SOURCES}
	 ${ALL_INCLUDES}
)


set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build)
#add_executable(demo_media_pusher_single ${ALL_SRCS})
add_executable(demo_media_pusher_single "demo_media_pusher_single.cpp")
target_link_libraries(demo_media_pusher_single ${LIB_MEDIA_PUSHER} ${LIST_FFMPEG})

add_executable(demo_media_finder_onvif "demo_media_finder_onvif.cpp")
target_link_libraries(demo_media_finder_onvif ${LIB_MEDIA_PUSHER} ${IPHLPAPI} ${LIST_OPENSSL_LIB})

add_executable(demo_media_finder_monitor "demo_media_finder_monitor.cpp")
target_link_libraries(demo_media_finder_monitor ${LIB_MEDIA_PUSHER} ${IPHLPAPI} ${LIST_OPENSSL_LIB})