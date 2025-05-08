FROM ros:noetic-ros-base

RUN apt-get update && \
    apt-get install -y libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev libavresample-dev libspqr2 libtbb-dev libopenblas-dev libcxsparse3

RUN apt-get install libglew-dev libopencv-dev -y
RUN apt-get install libboost-dev libboost-thread-dev libboost-filesystem-dev -y
RUN apt-get install cmake -y
RUN apt-get install build-essential -y
RUN apt install ros-noetic-cv-bridge ros-noetic-rviz ros-noetic-rviz-imu-plugin -y

RUN mkdir -p /ws
COPY ./Pangolin-0.6 /ws/Pangolin-0.6
RUN cd /ws/Pangolin-0.6 && mkdir build
RUN cd /ws/Pangolin-0.6/build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j && make install

CMD ["bash"]
