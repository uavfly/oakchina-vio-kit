cmake_minimum_required(VERSION 2.8.12)

set(APP_NAME oakchina_vio_demo)
project(${APP_NAME})

find_package( OpenCV REQUIRED )

link_directories(./lib)

add_executable(${APP_NAME}
    ./oakchina_vio_demo.cpp
)

target_link_libraries(${APP_NAME} 
    libcarina_vio.so
    libcarina_vio_client.so
    libcarina_client.so
    libclahe.a
    libDBoW2.a
    libg2o.a
    liblk.a
    libtracker.a
    libyaml-cpp.a
    ${OpenCV_LIBS}
)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
)
