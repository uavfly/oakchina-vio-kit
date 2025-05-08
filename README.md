# OAKChina-vio

## 支持平台

目前示例程序仅支持在Ubuntu20.04上编译运行

## 设置udev规则

```
sudo cp ./cpld.rules /etc/udev/rules.d
```

## 本地运行

### 安装依赖

```
chmod +x install_requirements.sh
./install_requirements.sh
```

### Build example

```
chmod +x build.sh
./build.sh
```

### Run example

```
./example/build/oakchina_vio_demo ./cusom_config.yaml ./database.bin
```

### Run ros example

```
source devel_isolated/setup.bash
roslaunch oakchina_vio_package oakchina_vio.launch
```

## Docker

使用docker可以在不同主机平台测试示例

### Build Docker Image

```
docker build -t oakchina_vio_kit .
```

### Run docker Container

```
xhost +local:docker

docker run -it --privileged --net=host --ipc=bridge --ipc=host --pid=host -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev/bus/usb:/dev/bus/usb -v $(pwd):/work oakchina_vio_kit /bin/bash
```

### Build example

```
chmod +x build.sh
./build.sh
```

### Run example

```
./example/build/oakchina_vio_demo ./cusom_config.yaml ./database.bin
```

### Run ros example

```
source /opt/ros/noetic/setup.bash
source devel_isolated/setup.bash
roslaunch oakchina_vio_package oakchina_vio.launch
```