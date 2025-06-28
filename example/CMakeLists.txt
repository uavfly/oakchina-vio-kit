cmake_minimum_required(VERSION 2.8.12)

set(APP_NAME oakchina_vio_demo)
project(${APP_NAME})

set(PACKAGE_NAME "oakchina_vio")
set(INSTALL_LIB_DIR lib/${PACKAGE_NAME})

message(STATUS "System processor: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
  set(LIB_ARCH_DIR "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
  set(LIB_ARCH_DIR "aarch64")
endif()

set(OPENCV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_ARCH_DIR})

link_directories(${LIB_DIR})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,${LIB_DIR}")

add_executable(${APP_NAME}
    ./oakchina_vio_demo.cpp
)

include_directories(
    include
    ${OPENCV_INCLUDE_DIR}
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
    libfpga_orb.a
    libopencv_calib3d.so.4.2
    libopencv_core.so.4.2
    libopencv_features2d.so.4.2
    libopencv_highgui.so.4.2
    libopencv_imgproc.so.4.2
    libopencv_video.so.4.2
    libopencv_imgcodecs.so.4.2
    libGLEW.so.2.1
)

set_target_properties(${APP_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "\$ORIGIN/../../${INSTALL_LIB_DIR}"
)

install(TARGETS ${APP_NAME}
    DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_ARCH_DIR}/
    DESTINATION ${INSTALL_LIB_DIR}
    USE_SOURCE_PERMISSIONS
)