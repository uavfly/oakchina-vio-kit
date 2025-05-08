#include "ros/ros.h"
#include "std_msgs/String.h"
#include "sensor_msgs/Imu.h"
#include "sensor_msgs/Image.h"
#include "geometry_msgs/PoseStamped.h"
#include <tf2/LinearMath/Quaternion.h>
#include <nav_msgs/Path.h>

#include <signal.h>
#include <atomic>
#include <fstream>
#include <iostream>
#include <vector>
#include <mutex>
#include <cstring>
#include <sstream>
#include "carina_a1088.h"
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

void signal_handle(int signal){
	if(SIGINT == signal){
		carina_a1088_pause();
        carina_a1088_stop();
        carina_a1088_release();
        exit(0);
	}
}

bool kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}

tf2::Quaternion rotationMatrixToQuaternion(const double* R) {
    tf2::Quaternion q;
    double trace = R[0] + R[4] + R[8]; // 列优先存储

    if (trace > 0) {
        double s = sqrt(trace + 1.0) * 2; // 4 * qw
        q.setW(0.25 * s);
        q.setX((R[7] - R[5]) / s);
        q.setY((R[2] - R[6]) / s);
        q.setZ((R[3] - R[1]) / s);
    } else {
        if (R[0] > R[4] && R[0] > R[8]) {
            double s = sqrt(1.0 + R[0] - R[4] - R[8]) * 2; // 4 * qx
            q.setW((R[7] - R[5]) / s);
            q.setX(0.25 * s);
            q.setY((R[1] + R[3]) / s);
            q.setZ((R[2] + R[6]) / s);
        } else if (R[4] > R[8]) {
            double s = sqrt(1.0 + R[4] - R[0] - R[8]) * 2; // 4 * qy
            q.setW((R[2] - R[6]) / s);
            q.setX((R[1] + R[3]) / s);
            q.setY(0.25 * s);
            q.setZ((R[7] + R[5]) / s);
        } else {
            double s = sqrt(1.0 + R[8] - R[0] - R[4]) * 2; // 4 * qz
            q.setW((R[3] - R[1]) / s);
            q.setX((R[2] + R[6]) / s);
            q.setY((R[7] + R[5]) / s);
            q.setZ(0.25 * s);
        }
    }

    return q;
}

int main(int argc, char **argv) {

    signal(SIGINT,signal_handle);

    char c;
    std::mutex image_mtx;
    double image_ts = -1;
    uint8_t *left_image = NULL;
    uint8_t *right_image = NULL;
    uint32_t width = 0, height = 0;

    std::mutex imu_mtx;
    std::vector<float> imu_data;
    double imu_ts = -1;

    std::mutex vsync_mtx;
    double vsync_ts = -1;

    std::mutex pose_mtx;
    float pose_data[32] = {0,};
    double pose_ts = -1;

    std::mutex points_mtx;
    carina_points points_data;
    double points_ts = -1;

    std::mutex event_mtx;
    unsigned char event = 0;

    std::string custom_config_path = "./custom_config.yaml";
    std::string custom_config;
    std::ifstream configFile(custom_config_path, std::ios::in);
    if (configFile.is_open()) {
        std::stringstream buffer;
        buffer << configFile.rdbuf();
        configFile.close();
        custom_config = buffer.str();
    }

    carina_a1088_init("", custom_config, "./database.bin");
    carina_a1088_start(
            [&](float *pose, double ts) {
                std::lock_guard<std::mutex> auto_lock(pose_mtx);
                memcpy(pose_data, pose, sizeof(float) * 32);
                pose_ts = ts;
            },
            [&](double ts) {
                std::lock_guard<std::mutex> auto_lock(vsync_mtx);
                vsync_ts = ts;
            },
            [&](const std::vector<float> &imu, double ts) {
                std::lock_guard<std::mutex> auto_lock(imu_mtx);
                // imu_data = imu;
                imu_ts = ts;
            },
            [&](const char *left, const char *right, const char *left1, const char *right1, double ts, int w, int h) {
                std::lock_guard<std::mutex> auto_lock(image_mtx);
                left_image = (uint8_t*)left;
                right_image = (uint8_t*)right;
                width = w;
                height = h;
                image_ts = ts;
            },
            [&](const carina_points &points, double ts) {
                std::lock_guard<std::mutex> auto_lock(points_mtx);
                points_data.points_lk = points.points_lk;
                points_data.points_orb = points.points_orb;
                points_ts = ts;
            },
            [&](const uint8_t uc_event) {
                std::lock_guard<std::mutex> auto_lock(event_mtx);
                event = uc_event;
            });
    carina_a1088_resume();

    ros::init(argc, argv, "vio");
    ros::NodeHandle n;

    // ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);
    // ros::Publisher imu_pub = n.advertise<sensor_msgs::Imu>("imu/data", 10);
    ros::Publisher pose_pub = n.advertise<geometry_msgs::PoseStamped>("pose", 10);
    ros::Publisher path_pub = n.advertise<nav_msgs::Path>("path", 10);
    ros::Publisher left_image_pub = n.advertise<sensor_msgs::Image>("left_gray_image", 10);
    ros::Publisher right_image_pub = n.advertise<sensor_msgs::Image>("right_gray_image", 10);
    ros::Rate loop_rate(10);
    nav_msgs::Path path;
    path.header.frame_id = "map";

    while (ros::ok()) {
        sensor_msgs::Imu imu_msg;
        geometry_msgs::PoseStamped pose_stamped;
        tf2::Quaternion quat;
        sensor_msgs::ImagePtr left_msg, right_msg;
        // std_msgs::String msg;
        // msg.data = "Hello, ROS!";
        {
            std::lock_guard<std::mutex> auto_lock(pose_mtx);
            if (pose_ts > 0) {
                // std::cout << "pose ts: " << pose_ts << " pose: " <<
                //           pose_data[12] << "\t" << pose_data[13]
                //           << "\t" << pose_data[14] << std::endl;
                // std::cout << "pose ts: " << pose_ts << " pose: ";
                // for (int i = 0; i < 16; i++) {
                //     std::cout << pose_data[i] << "\t";
                // }
                // std::cout << std::endl;

                double R[9] = {
                    pose_data[0], pose_data[1], pose_data[2],
                    pose_data[4], pose_data[5], pose_data[6],
                    pose_data[8], pose_data[9], pose_data[10]
                };

                quat = rotationMatrixToQuaternion(R);
                pose_stamped.header.stamp = ros::Time::now();
                pose_stamped.header.frame_id = "map"; // 或其他适当的坐标框架
                pose_stamped.pose.position.x = pose_data[12];
                pose_stamped.pose.position.y = pose_data[13];
                pose_stamped.pose.position.z = pose_data[14];
                pose_stamped.pose.orientation.x = quat.x();
                pose_stamped.pose.orientation.y = quat.y();
                pose_stamped.pose.orientation.z = quat.z();
                pose_stamped.pose.orientation.w = quat.w();

                path.poses.push_back(pose_stamped);
                path.header.stamp = ros::Time::now(); // 更新时间戳
                pose_ts = -1;
            }
        }

        {
            std::lock_guard<std::mutex> auto_lock(imu_mtx);
            if (imu_ts > 0) {
                // std::cout << "imu ts: " << imu_ts << " data: " <<
                //           imu_data[0] << ", " << imu_data[1] << ", " << imu_data[2] << ", " <<
                //           imu_data[3] << ", " << imu_data[4] << ", " << imu_data[5] << std::endl;
                // imu_msg.header.stamp = ros::Time::now(); // 当前时间
                // imu_msg.header.frame_id = "map";
                // imu_msg.linear_acceleration.x = imu_data[0]; // X 轴加速度
                // imu_msg.linear_acceleration.y = imu_data[1]; // Y 轴加速度
                // imu_msg.linear_acceleration.z = imu_data[2]; // Z 轴加速度（重力）

                // imu_msg.angular_velocity.x = imu_data[3]; // X 轴角速度
                // imu_msg.angular_velocity.y = imu_data[4]; // Y 轴角速度
                // imu_msg.angular_velocity.z = imu_data[5]; // Z 轴角速度

                // imu_msg.orientation.x = quat.x(); // 四元数 X
                // imu_msg.orientation.y = quat.y(); // 四元数 Y
                // imu_msg.orientation.z = quat.z(); // 四元数 Z
                // imu_msg.orientation.w = quat.w(); // 四元数 W
                // imu_msg.orientation_covariance = {-1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
                imu_ts = -1;
            }
        }

        {
            std::lock_guard<std::mutex> auto_lock(vsync_mtx);
            if (vsync_ts > 0) {
                std::cout << "vsync ts: " << vsync_ts << std::endl;
                vsync_ts = -1;
            }
        }

        {
            std::lock_guard<std::mutex> auto_lock(points_mtx);
            if (points_ts > 0) {
                std::cout << "points ts: " << points_ts << " data: " << points_data.points_orb.size() << "\t"
                          << points_data.points_lk.size() << std::endl;

                points_ts = -1;
            }
        }

        {
            std::lock_guard<std::mutex> auto_lock(image_mtx);
            if (image_ts > 0 && left_image && right_image) {
                std::cout << "image_ts ts: " << image_ts << std::endl;
                cv::Mat left_img(height, width, CV_8UC1, left_image);
                left_msg = cv_bridge::CvImage(std_msgs::Header(), "mono8", left_img).toImageMsg();
                cv::Mat right_img(height, width, CV_8UC1, right_image);
                right_msg = cv_bridge::CvImage(std_msgs::Header(), "mono8", right_img).toImageMsg();
                left_image_pub.publish(left_msg);
                right_image_pub.publish(right_msg);
                image_ts = -1;
                left_image = NULL;
                right_image = NULL;
            }
        }

        // chatter_pub.publish(msg);
        pose_pub.publish(pose_stamped);
        // imu_pub.publish(imu_msg);

        path_pub.publish(path);
        ros::spinOnce();
        loop_rate.sleep();

        if (kbhit())
        {
            c = fgetc(stdin);
            std::cout << std::endl;
            if (c == 'q')
                break;
        }
    }

    carina_a1088_pause();
    carina_a1088_stop();
    carina_a1088_release();

    return 0;
}