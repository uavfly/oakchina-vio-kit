#!/bin/bash
set -euo pipefail  # 严格错误处理：遇错终止、未定义变量报错、管道错误传递

# 获取脚本所在目录，确保路径稳定
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR"  # 切换到项目根目录

# --------------------------
# 0. 预检查必要环境
# --------------------------
echo "🔍 检查构建环境..."
if [[ -n "${ROS_DISTRO:-}" ]]; then
    if [[ "$ROS_DISTRO" != "noetic" ]]; then
        echo "⚠️ 检测到 ROS 版本非 Noetic（当前: $ROS_DISTRO），可能导致兼容问题！"
    fi
else
    echo "⚠️ 未激活 ROS 环境（未设置 ROS_DISTRO），可能影响 ROS 示例构建！"
    echo "   请尝试执行: source /opt/ros/noetic/setup.bash"
fi

# --------------------------
# 1. 构建 Pangolin-0.6
# --------------------------
echo "🚀 开始构建 Pangolin-0.6 ..."
cd "Pangolin-0.6" || { echo "❌ 找不到 Pangolin-0.6 目录"; exit 1; }

# 清理旧构建目录（可选，保留注释供用户选择）
# rm -rf build  # 谨慎：会删除历史构建文件

mkdir -p build && cd build
echo "  执行 cmake ..."
cmake -DCMAKE_BUILD_TYPE=Release ..
echo "  执行 make（并行数：$(nproc)）..."
make -j$(nproc)
echo "  安装 Pangolin ..."
sudo make install
cd "$SCRIPT_DIR"  # 返回项目根目录
echo "✅ Pangolin-0.6 构建并安装完成"

# --------------------------
# 2. 构建示例程序
# --------------------------
echo "🚀 开始构建示例程序 ..."
cd "example" || { echo "❌ 找不到 example 目录"; exit 1; }

mkdir -p build && cd build
echo "  执行 cmake ..."
cmake ..
echo "  执行 make（并行数：$(nproc)）..."
make -j$(nproc)
cd "$SCRIPT_DIR"  # 返回项目根目录
echo "✅ 示例程序构建完成"

# --------------------------
# 3. 构建 ROS 示例
# --------------------------

build_ros1_example() {
    ROS_WS_DIR="ros-example/catkin_ws"
    cd "$ROS_WS_DIR" || { echo "❌ 找不到 ROS 工作空间目录: $ROS_WS_DIR"; exit 1; }

    echo "  安装 ROS 依赖 ..."
    rosdep install --from-paths src --ignore-src -r -y
    echo "  执行 catkin_make_isolated ..."
    catkin_make_isolated -j$(nproc)
}

build_ros2_example() {
    ROS_WS_DIR="ros-example/oakchina_vio_package_ros2"
    cd "$ROS_WS_DIR" || { echo "❌ 找不到 ROS2 工作空间目录: $ROS_WS_DIR"; exit 1; }

    echo "  安装 ROS 依赖 ..."
    rosdep init
    rosdep update
    rosdep install --from-paths src --ignore-src -r -y
    echo "  执行 colcon build ..."
    colcon build --symlink-install
}

echo "🚀 开始构建 ROS 示例 ..."
if [ -f /etc/os-release ]; then
    source /etc/os-release
    if [ "$ID" = "ubuntu" ]; then
        case "$VERSION_ID" in
            "20.04")
                echo "This is Ubuntu 20.04 (Focal Fossa)"
                build_ros1_example
                ;;
            "22.04")
                echo "This is Ubuntu 22.04 (Jammy Jellyfish)"
                build_ros2_example
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

cd "$SCRIPT_DIR"  # 返回项目根目录
echo "✅ ROS 示例构建完成"

echo "🎉 所有构建任务完成！"
