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

std::mutex image_mtx;
cv::Mat left_image;
cv::Mat right_image;
cv::Mat left_image1;
cv::Mat right_image1;
double image_ts = -1;

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

void CarinaA1088PoseCallBack(float *pose, double ts) {
    std::lock_guard<std::mutex> auto_lock(pose_mtx);
    memcpy(pose_data, pose, sizeof(float) * 32);
    pose_ts = ts;
}

void CarinaA1088VsyncCallBack(double ts) {
    std::lock_guard<std::mutex> auto_lock(vsync_mtx);
    vsync_ts = ts;
}

void CarinaA1088ImuCallBack(float *imu, double ts) {
    std::lock_guard<std::mutex> auto_lock(imu_mtx);
    if (imu != nullptr) {
        imu_data.resize(6);
        for (int i = 0; i < 6; i++) {
            imu_data[i] = imu[i];
        }
    }
    imu_ts = ts;
}

void CarinaA1088CameraCallBack(char *left, char *right, char *left1, char *right1, double ts, int w, int h) {
    std::lock_guard<std::mutex> auto_lock(image_mtx);
    if (left != nullptr) {
        left_image = cv::Mat(h, w, CV_8UC1);
        memcpy(left_image.data, left, w * h);
    }
    if (right != nullptr) {
        right_image = cv::Mat(h, w, CV_8UC1);
        memcpy(right_image.data, right, w * h);
    }
    if (left1 != nullptr) {
        left_image1 = cv::Mat(h, w, CV_8UC1);
        memcpy(left_image1.data, left1, w * h);
    }
    if (right1 != nullptr) {
        right_image1 = cv::Mat(h, w, CV_8UC1);
        memcpy(right_image1.data, right1, w * h);
    }
    image_ts = ts;
}

void CarinaA1088PointsCallBack(carina_points &points, double ts) {
    std::lock_guard<std::mutex> auto_lock(points_mtx);
    for (int i = 0; i < points_data.points_lk_rows; ++i) {
        delete[] points_data.points_lk[i];
    }
    delete[] points_data.points_lk;
    for (int i = 0; i < points_data.points_orb_rows; ++i) {
        delete[] points_data.points_orb[i];
    }
    delete[] points_data.points_orb;
    points_data.points_lk_rows = points.points_lk_rows;
    points_data.points_orb_rows = points.points_orb_rows;

    points_data.points_lk = new carina_lk_point *[points_data.points_lk_rows];
    for (int i = 0; i < points_data.points_lk_rows; ++i) {
        points_data.points_lk_cols[i] = points.points_lk_cols[i];
        points_data.points_lk[i] = new carina_lk_point[points_data.points_lk_cols[i]];
        for (int j = 0; j < points_data.points_lk_cols[i]; ++j) {
            points_data.points_lk[i][j].id = points.points_lk[i][j].id;
            points_data.points_lk[i][j].x = points.points_lk[i][j].x;
            points_data.points_lk[i][j].y = points.points_lk[i][j].y;
        }
    }

    points_data.points_orb = new carina_orb_point *[points_data.points_orb_rows];
    for (int i = 0; i < points_data.points_orb_rows; ++i) {
        points_data.points_orb_cols[i] = points.points_orb_cols[i];
        points_data.points_orb[i] = new carina_orb_point[points_data.points_orb_cols[i]];
        for (int j = 0; j < points_data.points_orb_cols[i]; ++j) {
            points_data.points_orb[i][j].id = points.points_orb[i][j].id;
            points_data.points_orb[i][j].x = points.points_orb[i][j].x;
            points_data.points_orb[i][j].y = points.points_orb[i][j].y;
            points_data.points_orb[i][j].angle = points.points_orb[i][j].angle;
            points_data.points_orb[i][j].octave = points.points_orb[i][j].octave;
            points_data.points_orb[i][j].response = points.points_orb[i][j].response;
            for (int k = 0; k < 32; ++k) {
                points_data.points_orb[i][j].desc[k] = points.points_orb[i][j].desc[k];
            }
        }
    }
    points_ts = ts;
}

void CarinaA1088EventCallBack(const uint8_t uc_event) {
    std::lock_guard<std::mutex> auto_lock(event_mtx);
    event = uc_event;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        std::cout << "Please set config" << std::endl;
        return 0;
    }

    std::string config_file_path = argv[1];
    char *database_file_path = argv[2];
    std::string custom_config;
    std::ifstream configFile(config_file_path, ios::in);
    if (configFile.is_open()) {
        std::stringstream buffer;
        buffer << configFile.rdbuf();
        configFile.close();
        custom_config = buffer.str();
    } else {
        std::cout << "Please set the correct custom_config.yaml path" << std::endl;
    }

    carina_a1088_init(const_cast<char *>(custom_config.c_str()), database_file_path);
    carina_a1088_start(
        CarinaA1088PoseCallBack,
        CarinaA1088VsyncCallBack,
        CarinaA1088ImuCallBack,
        CarinaA1088CameraCallBack,
        CarinaA1088PointsCallBack,
        CarinaA1088EventCallBack
    );
    carina_a1088_resume();

    std::string cam_param = carina_a1088_get_cam_param();
    std::cout << "cam_param: " << std::endl << cam_param << std::endl;
    std::cout << "------------------------" << std::endl;

    std::string sn = carina_a1088_get_sn();
    std::cout << "sn: " << std::endl << sn << std::endl;
    std::cout << "------------------------" << std::endl;

    std::string sdk_version = carina_a1088_get_sdk_version();
    std::cout << "sdk_version: " << std::endl << sdk_version << std::endl;
    std::cout << "------------------------" << std::endl;

    std::string firmware_version = carina_a1088_get_firmware_version();
    std::cout << "firmware_version: " << std::endl << firmware_version << std::endl;
    std::cout << "------------------------" << std::endl;

    cv::Mat left_image_show;
    cv::Mat right_image_show;
    cv::Mat left_image_show1;
    cv::Mat right_image_show1;
    bool image_show = true;
    if (argc >= 4) {
        std::string str_image_show = argv[3];
        if (str_image_show == "0")
            image_show = false;
    }

    char c;
    while (1) {
        {
            std::lock_guard<std::mutex> auto_lock(pose_mtx);
            if (pose_ts > 0) {
                double R[9] = {
                    pose_data[0], pose_data[4], pose_data[8],
                    pose_data[1], pose_data[5], pose_data[9],
                    pose_data[2], pose_data[6], pose_data[10]
                };

                std::cout << "rotation matrix: ";
                for (size_t i = 0; i < 9; i++)
                {
                    std::cout << R[i] << "\t";
                }
                std::cout << std::endl;
                std::cout << "pose ts: " << pose_ts << " pose: " <<
                          pose_data[12] << "\t" << pose_data[13]
                          << "\t" << pose_data[14] << std::endl;
                
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

        // {
        //     std::lock_guard<std::mutex> auto_lock(vsync_mtx);
        //     if (vsync_ts > 0) {
        //         std::cout << "vsync ts: " << vsync_ts << std::endl;
        //         vsync_ts = -1;
        //     }
        // }

        // {
        //     std::lock_guard<std::mutex> auto_lock(points_mtx);
        //     if (points_ts > 0) {
        //         std::cout << "points ts: " << points_ts << " data: " << points_data.points_orb.size() << "\t"
        //                   << points_data.points_lk.size() << std::endl;

        //         points_ts = -1;
        //     }
        // }

        {
            std::lock_guard<std::mutex> auto_lock(image_mtx);
            if (image_ts > 0) {
                left_image_show = left_image.clone();
                right_image_show = right_image.clone();
                left_image_show1 = left_image1.clone();
                right_image_show1 = right_image1.clone();
                image_ts = -1;
            }
        }

        if (image_show) {
            if (!left_image_show.empty())
                cv::imshow("left", left_image_show);
            if (!right_image_show.empty())
                cv::imshow("right", right_image_show);
            if (!left_image_show1.empty())
                cv::imshow("left1", left_image_show1);
            if (!right_image_show1.empty())
                cv::imshow("right1", right_image_show1);

            if (cv::waitKey(1) == 'q') {
                break;
            }
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