# Pangolin 0.6

Pangolin是一个轻量级的跨平台C++开发库，主要用于管理OpenGL显示/上下文以及与基于图像和视频流的输入设备进行交互。它还提供了一些基础的GUI功能用于创建和管理窗口、处理鼠标和键盘事件等。

## 特性

- 跨平台窗口管理（支持Linux, macOS, Windows）
- OpenGL上下文管理
- 图像和视频流处理
- 基础GUI组件
- 支持多种视频输入设备和格式
- 提供简单的绘图和可视化工具

## 安装

### 依赖项

- CMake
- OpenGL
- GLUT (或FreeGLUT)
- GLEW
- 可选：Python支持（用于Python绑定）

### 构建步骤

```bash
git clone https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake ..
make -j
sudo make install
```

## 使用示例

### C++ 示例

```cpp
#include <pangolin/pangolin.h>

int main( int /*argc*/, char** /*argv*/ )
{
    // 创建一个窗口
    pangolin::CreateWindowAndBind("Main", 640, 480);
    
    // 设置OpenGL上下文
    glEnable(GL_DEPTH_TEST);
    
    // 主循环
    while( !pangolin::ShouldQuit() )
    {
        // 清除屏幕和深度缓冲区
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 交换前后缓冲区并处理事件
        pangolin::FinishFrame();
    }
    
    return 0;
}
```

### Python 示例

```python
import pypangolin as pangolin
from OpenGL import GL

def main():
    pangolin.CreateWindowAndBind('Main', 640, 480)
    GL.glEnable(GL.GL_DEPTH_TEST)

    while not pangolin.ShouldQuit():
        GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)
        pangolin.FinishFrame()

if __name__ == '__main__':
    main()
```

## 文档

完整的文档和API参考请查看[Pangolin官方文档](https://github.com/stevenlovegrove/Pangolin)。

## 贡献

欢迎贡献代码和文档。请先阅读贡献指南并提交PR。

## 许可证

Pangolin使用MIT许可证。详情请查看[LICENSE](LICENCE)文件。