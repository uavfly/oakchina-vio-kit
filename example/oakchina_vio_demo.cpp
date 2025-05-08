#include <atomic>
#include <fstream>
#include <iostream>
#include <vector>
#include <mutex>
#include <cstring>
#include <sstream>
#include "carina_a1088.h"
#ifndef WIN32
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>

#else
#include <conio.h>
#endif

#include <opencv2/opencv.hpp>

using namespace std;

#ifndef WIN32
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
#endif

int main(int argc, char **argv) {
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
    std::ifstream configFile(custom_config_path, ios::in);
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
                imu_data = imu;
                imu_ts = ts;
            },
            [&](const char *left, const char *right, const char *left1, const char *right1, double ts, int w, int h) {
                std::lock_guard<std::mutex> auto_lock(image_mtx);
                image_ts = ts;
                left_image = (uint8_t*)left;
                right_image = (uint8_t*)right;
                width = w;
                height = h;
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

    char c;
    cv::Mat left_img(height, width, CV_8UC1);
    cv::Mat right_img(height, width, CV_8UC1);
    while (1) {
        {
            std::lock_guard<std::mutex> auto_lock(pose_mtx);
            if (pose_ts > 0) {
                std::cout << "pose ts: " << pose_ts << " pose: " <<
                          pose_data[12] << "\t" << pose_data[13]
                          << "\t" << pose_data[14] << std::endl;
                // std::cout << "pose ts: " << pose_ts << " pose: ";
                // for (int i = 0; i < 16; i++) {
                //     std::cout << pose_data[i] << "\t";
                // }
                // std::cout << std::endl;
                pose_ts = -1;
            }
        }

        {
            std::lock_guard<std::mutex> auto_lock(imu_mtx);
            if (imu_ts > 0) {
                std::cout << "imu ts: " << imu_ts << " data: " <<
                          imu_data[0] << ", " << imu_data[1] << ", " << imu_data[2] << ", " <<
                          imu_data[3] << ", " << imu_data[4] << ", " << imu_data[5] << std::endl;
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
            if (image_ts > 0) {
                std::cout << "image_ts ts: " << image_ts << std::endl;
                cv::Mat original_left_img(height, width, CV_8UC1, left_image);
                cv::Mat original_right_img(height, width, CV_8UC1, right_image);
                left_img = original_left_img.clone();
                right_img = original_right_img.clone();
                left_image = NULL;
                right_image = NULL;
                image_ts = -1;
            }
        }

        if (!left_img.empty() && !right_img.empty()) {
            cv::imshow("left", left_img);
            cv::imshow("right", right_img);
            cv::waitKey(1);
        }

#ifndef WIN32
        if (kbhit())
        {
            c = fgetc(stdin);
            cout << endl;
            if (c == 'q')
                break;                
        }
#else
        if (_kbhit())
        {
            c = _getch();
            cout << endl;
            if (c == 'q')
                break;
        }
#endif

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    cv::destroyAllWindows();
    carina_a1088_pause();
    carina_a1088_stop();
    carina_a1088_release();
    return EXIT_SUCCESS;
}