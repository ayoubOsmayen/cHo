cmake_minimum_required(VERSION 3.10)
project(rtspServer)

# Set C standard to 99
set(CMAKE_C_STANDARD 99)

# Manually specify the include and library directories
set(GSTREAMER_INCLUDE_DIR "/usr/local/include/gstreamer-1.0")
set(GSTREAMER_LIB_DIR "/usr/local/lib/x86_64-linux-gnu")

# Set include and library directories manually
include_directories(
    ${GSTREAMER_INCLUDE_DIR}
    /usr/include/mariadb
    /usr/include/glib-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
)

link_directories(
    ${GSTREAMER_LIB_DIR}
    /usr/lib/x86_64-linux-gnu
)

# Find GStreamer and MariaDB packages using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-rtsp-server-1.0)

# Define the source files for your project
set(SOURCES
    main.c
    functions.c
    config.c
)

# Create the executable target
add_executable(rtspServer ${SOURCES})

# Link GStreamer and MariaDB client libraries
target_link_libraries(rtspServer 
    ${GST_LIBRARIES}  # GStreamer libraries
    mariadbclient     # MariaDB client library
)
