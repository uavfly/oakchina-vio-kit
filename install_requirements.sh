#!/bin/bash

 apt update
 apt install -y libavcodec-dev libavformat-dev libswscale-dev libspqr2 libtbb-dev libopenblas-dev libcxsparse3
 apt install libglew-dev libopenblas-dev libopencv-dev -y
 apt install libboost-dev libboost-thread-dev libboost-filesystem-dev -y
 apt install cmake -y
 apt install build-essential -y

install_ros1_depend() {
     apt install libdc1394-22-dev -y
     apt install libavresample-dev -y
     sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
     apt install curl
    curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc |  apt-key add -

     apt update
     apt install ros-noetic-desktop -y
     apt install python3-rosdep -y
     rosdep init
    rosdep update
     apt install ros-noetic-cv-bridge ros-noetic-rviz ros-noetic-rviz-imu-plugin -y
}

install_ros2_depend() { 
     apt install libdc1394-dev -y
     apt install software-properties-common -y
     apt-get install libswresample-dev -y
     add-apt-repository universe
     apt update &&  apt install curl -y
    export ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F\" '{print $4}')
    curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo $VERSION_CODENAME)_all.deb" # If using Ubuntu derivates use $UBUNTU_CODENAME
     dpkg -i /tmp/ros2-apt-source.deb
     apt update

     apt install ros-humble-ros-base -y
     apt install ros-dev-tools -y
     apt install ros-humble-cv-bridge ros-humble-rviz2 ros-humble-rviz-imu-plugin -y
     rosdep init
    rosdep update
}

if [ -f /etc/os-release ]; then
    source /etc/os-release
    if [ "$ID" = "ubuntu" ]; then
        case "$VERSION_ID" in
            "20.04")
                echo "This is Ubuntu 20.04 (Focal Fossa)"
                install_ros1_depend
                ;;
            "22.04")
                echo "This is Ubuntu 22.04 (Jammy Jellyfish)"
                install_ros2_depend
                ;;
            *)
                echo "This is Ubuntu, but not 20.04 or 22.04"
                ;;
        esac
    else
        echo "This is not Ubuntu"
    fi
else
    echo "Cannot determine OS (no /etc/os-release)"
fi
