#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <vector>
#include <string.h>
#include <ctime>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class camera {

    int index;
    int sensitivity;
    int camera_index;
    cv::VideoCapture video_capture;

    public:

    
    std::vector<std::string> grab_images();
    int monitor();

    camera(int sensitivity, int index, int camera_index);

};
