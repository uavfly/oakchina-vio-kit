FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /ws

RUN apt update
RUN apt install -y libavcodec-dev libavformat-dev libswscale-dev libspqr2 libtbb-dev libopenblas-dev libcxsparse3
RUN apt install libglew-dev libopenblas-dev libopencv-dev -y
RUN apt install libboost-dev libboost-thread-dev libboost-filesystem-dev -y
RUN apt install cmake -y
RUN apt install build-essential -y

RUN apt install libdc1394-dev -y
RUN apt install software-properties-common -y
RUN apt-get install libswresample-dev -y
RUN add-apt-repository universe
RUN apt update &&  apt install curl -y
RUN curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/1.1.0/ros2-apt-source_1.1.0.$(. /etc/os-release && echo $VERSION_CODENAME)_all.deb"
RUN dpkg -i /tmp/ros2-apt-source.deb
RUN apt update

RUN apt install ros-humble-ros-base -y
RUN apt install ros-dev-tools -y
RUN apt install ros-humble-cv-bridge ros-humble-rviz2 ros-humble-rviz-imu-plugin -y

RUN apt install git -y
RUN git clone --branch 4.2.0 https://github.com/opencv/opencv.git
RUN apt-get install python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev
RUN cd opencv && cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -Bbuild  -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_opencv_apps=OFF -D WITH_GTK=ON -DWITH_GTK_2_X=OFF .
RUN cd opencv && cmake --build build --target install

COPY ./Pangolin-0.6 ./Pangolin-0.6
RUN cd ./Pangolin-0.6 && mkdir build
RUN cd ./Pangolin-0.6/build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j6 && make install