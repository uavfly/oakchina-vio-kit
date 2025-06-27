cmake_minimum_required(VERSION 3.5)
project(oakchina_vio_package)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexceptions" )

#set(OpenCV_DIR /home/uavpub/opencv-4.2.0/build/)  # 如果不把a1088SDK的动态链库更换成静态链库，在非opencv4.2版本下编译会报错，需要手动编译并指定OpenCV的路径

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

include_directories(
  include
  ${OpenCV_INCLUDE_DIRS}
)

message(STATUS "System processor: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
  link_directories(lib/x86_64)
  add_executable(${PROJECT_NAME}_node src/main.cpp)
  # 目标链接库
  target_link_libraries(${PROJECT_NAME}_node
    ${OpenCV_LIBS}
    libcarina_vio.so
    libcarina_vio_client.so
    libcarina_client.so
  )
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
  link_directories(lib/aarch64)
  add_executable(${PROJECT_NAME}_node src/main.cpp)
  target_link_libraries(${PROJECT_NAME}_node
    ${catkin_LIBRARIES}
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
    ${OpenCV_LIBS}
  )
endif()

ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  std_msgs
  sensor_msgs
  cv_bridge
  nav_msgs
  geometry_msgs
  tf2_ros
  tf2_geometry_msgs
  tf2
)

install(TARGETS ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME}
)

file(GLOB_RECURSE DEPENDENCY_LIBS 
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.so*"
)

install(FILES ${DEPENDENCY_LIBS}
  DESTINATION lib
)

# 安装其他资源文件
install(DIRECTORY launch config rviz
  DESTINATION share/${PROJECT_NAME}
)

ament_export_dependencies(
  rclcpp
  std_msgs
  sensor_msgs
  cv_bridge
  nav_msgs
  geometry_msgs
  tf2_ros
  tf2_geometry_msgs
  tf2
)

ament_export_include_directories(include)

ament_export_libraries(${PROJECT_NAME}_node)

ament_package()