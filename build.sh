#!/bin/bash

# build Pangolin-0.6

cd Pangolin-0.6
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. && make -j
sudo make install
cd ../..

# build example

cd example
mkdir build && cd build
cmake .. && make -j
cd ../..

# build ros example

cd ros-example/catkin_ws

rosdep install --from-paths src --ignore-src -r -y
catkin_make_isolated