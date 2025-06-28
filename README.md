# OAKChina-vio 

视觉惯性里程计（VIO）示例项目，支持 **本地环境** 和 **Docker 容器** 快速部署。
本人oakchina-vio-kit Ubuntu22.04以及ROS2-Humble适配第一作者（已授权官方使用）。
官方Gitee地址：https://gitee.com/oakchina/oakchina-vio

---

## 📋 目录
1. [支持平台](#-支持平台)
2. [前置准备](#-前置准备)
3. [本地运行](#-本地运行)
4. [Docker 运行](#-docker-运行)
5. [注意事项](#-注意事项)

---

## 🖥️ 支持平台
| 环境         | 版本               | 
|--------------|--------------------|
| **操作系统**  | Ubuntu 20.04 LTS   | 
| **操作系统**  | Ubuntu 22.04 LTS   | 
| **ROS**      | Noetic Ninjemys    | 
| **ROS2**     | Humble Hawksbill  |

---

## 🔧 前置准备

### 1. 设置 udev 规则
确保 OAK 设备可被正确识别：
```bash
sudo cp ./cpld.rules /etc/udev/rules.d
sudo udevadm control --reload-rules && sudo udevadm trigger  # 重启 udev 服务
```

---

## 💻 本地运行

### 1. 安装依赖
通过脚本一键安装（需联网）：
```bash
chmod +x install_requirements.sh
./install_requirements.sh  # 自动安装 OpenCV4、Boost、ROS Noetic 等
```

### 2. 构建项目

####
* Tips! : Update 2025.06.29 更新后opencv4.2.0相关头文件以及动态链接库已经被包含于项目中，因此不再需要手动编译opencv4.2.0！
* 此项目已经在X86（Intel I3-N305、I7-12700H）以及AARCH64（Rockchip RK3588）平台上测试通过（均为Ubuntu22.04 + Ros2 Humble Hawksbill 系统），理论上支持所有x86_64以及aarch64架构的平台。
####

<del>

```bash
# 在Ubuntu22.04系统上，需要手动编译opencv v4.2
git clone --branch 4.2.0 https://github.com/opencv/opencv.git
sudo apt-get install python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev
cd opencv
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -Bbuild  -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_opencv_apps=OFF .
sudo cmake --build build --target install
```

</del>

编译ROS功能包之前请确保已经正确加载相关ROS环境。

```bash
source /opt/ros/noetic/setup.bash          # 加载基础 ROS 环境
source /opt/ros/humble/setup.bash          # 加载基础 ROS2 环境
chmod +x build.sh
./build.sh  # 编译主程序及 ROS 功能包
```

### 3. 运行示例程序
#### 非 ROS 模式
```bash
./example/build/oakchina_vio_demo ./custom_config.yaml ./database.bin
```

#### ROS 模式
```bash
cd ros-example/catkin_ws

# 激活 ROS 环境（每个新终端都需要执行）
source /opt/ros/noetic/setup.bash          # 加载基础 ROS 环境
source devel_isolated/setup.bash           # 加载项目 ROS 环境

# 验证环境变量
echo $ROS_PACKAGE_PATH | grep "oakchina_vio_package"  # 应显示项目路径

# 启动 VIO 节点
roslaunch oakchina_vio_package oakchina_vio.launch
```

#### ROS2 模式
```bash
cd ros-example/oakchina_vio_package_ros2

# 激活 ROS 环境（每个新终端都需要执行）
source /opt/ros/humble/setup.bash          # 加载基础 ROS 环境
source install/setup.bash                  # 加载项目 ROS2 环境

ros2 launch oakchina_vio_package oakchina_vio.launch.py
```
---

## 🐳 Docker 运行

### 1. 构建镜像
```bash
docker build -t oakchina_vio_kit .  # 包含完整依赖链的轻量级镜像
```

### 2. 启动容器
```bash
xhost +local:docker  # 允许容器访问 X11 显示

docker run -it --privileged \
  --net=host \
  -e DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /dev/bus/usb:/dev/bus/usb \
  -v $(pwd):/work \
  oakchina_vio_kit /bin/bash
```

### 3. 构建项目
```bash
source /opt/ros/noetic/setup.bash          # 加载基础 ROS 环境
source /opt/ros/humble/setup.bash          # 加载基础 ROS2 环境
# 构建项目（同本地）
chmod +x build.sh
./build.sh
```
#### 非 ROS 模式
```bash
./example/build/oakchina_vio_demo ./custom_config.yaml ./database.bin
```

#### ROS 模式
```bash
cd ros-example/catkin_ws

# 激活 ROS 环境（每个新终端都需要执行）
source /opt/ros/noetic/setup.bash          # 加载基础 ROS 环境
source devel_isolated/setup.bash           # 加载项目 ROS 环境

# 验证环境变量
echo $ROS_PACKAGE_PATH | grep "oakchina_vio_package"  # 应显示项目路径

# 启动 VIO 节点
roslaunch oakchina_vio_package oakchina_vio.launch
```
#### ROS2 模式
```bash
cd ros-example/oakchina_vio_package_ros2

# 激活 ROS 环境（每个新终端都需要执行）
source /opt/ros/humble/setup.bash          # 加载基础 ROS 环境
source install/setup.bash           # 加载项目 ROS 环境

# 验证环境变量
echo $ROS_PACKAGE_PATH | grep "oakchina_vio_package"  # 应显示项目路径

# 启动 VIO 节点
roslaunch oakchina_vio_package oakchina_vio.launch
```
---

## ⚠️ 注意事项

### 通用问题
- **设备未识别** → 检查 `cpld.rules` 是否生效，重启 udev 服务。
- **Docker 显示异常** → 确认执行 `xhost +local:docker`。
- **依赖安装失败** → 使用 `apt update && apt upgrade` 更新系统后再试。

### ROS 相关问题
- **环境冲突** → 确保未激活其他 ROS 版本（如 Melodic）。
- **launch 文件错误** → 检查 `catkin_ws` 是否完整编译。

### 性能优化
- 在 `custom_config.yaml` 中调整图像分辨率和帧率。
- Docker 模式下建议分配至少 4GB 内存。

## 贡献者
| 项目                             | 贡献者                                                    | 
|--------------------------------- |----------------------------------------------------------|
| oakchina_vio_package_ros2       | <a href="https://github.com/uavfly">StrangeFly</a> |