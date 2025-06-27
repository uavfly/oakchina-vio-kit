import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import Shutdown

def generate_launch_description():

    pkg_dir = get_package_share_directory('oakchina_vio_package')
    
    custom_config = os.path.join(pkg_dir, 'config', 'custom_config.yaml')
    database_path = os.path.join(pkg_dir, 'config', 'database.bin')
    rviz_config = os.path.join(pkg_dir, 'rviz', 'oakchina_vio.rviz')
    
    vio_node = Node(
        package='oakchina_vio_package',
        executable='oakchina_vio_package_node',
        name='oakchina_vio',
        output='screen',
        parameters=[
            {'custom_config_path': custom_config},
            {'database_path': database_path}
        ],
        on_exit=Shutdown()
    )
    
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz',
        arguments=['-d', rviz_config],
        output='screen'
    )
    
    return LaunchDescription([
        vio_node,
        rviz_node
    ])