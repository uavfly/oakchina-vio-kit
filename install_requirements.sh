#!/bin/bash

sudo apt update
sudo apt install -y libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev libavresample-dev libspqr2 libtbb-dev libopenblas-dev libcxspar
sudo apt install libglew-dev libopencv-dev -y
sudo apt install libboost-dev libboost-thread-dev libboost-filesystem-dev -y
sudo apt install cmake -y
sudo apt install build-essential -y

sudo apt install python3-rosdep
sudo rosdep init
rosdep update
sudo apt install ros-noetic-cv-bridge ros-noetic-rviz ros-noetic-rviz-imu-plugin -y