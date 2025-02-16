cmake_minimum_required(VERSION 3.10)
project(RTSP_Proxy)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-rtsp-server-1.0)

add_executable(serv_rtsp main.c)
target_include_directories(serv_rtsp PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(serv_rtsp ${GST_LIBRARIES})
target_compile_options(serv_rtsp PRIVATE ${GST_CFLAGS_OTHER})
