#include "camera.hpp"

int camera::pop_camera(){

    images.pop_back();
    return 1;

}

cv::Mat camera::get_next_plate(){

    return images.back();
}

int camera::monitor() {

    cv::Mat frame1, frame2;
    float dist;
    clock_t ticks1, ticks2;
    float threshold = 50.0f;
    cv::Mat diffImage;
    int pixdif = 0;
    //default 65000
    while(pixdif < 10000){

        ticks1=clock();

        ticks2=ticks1;

        //wait between every initial capture
        while((ticks2/CLOCKS_PER_SEC-ticks1/CLOCKS_PER_SEC)< .1 ) ticks2=clock();

        this->video_capture >> frame1;

        while((ticks2/CLOCKS_PER_SEC-ticks1/CLOCKS_PER_SEC)<.002) ticks2=clock();

        pixdif = 0;
        this->video_capture >> frame2;

        cv::absdiff(frame1, frame2, diffImage);

        cv::Mat foregroundMask = cv::Mat::zeros(diffImage.rows, diffImage.cols, CV_8UC1);


        //printf("\n %d  %d\n",diffImage.rows, diffImage.cols );
        for(int j=diffImage.rows/3; j<(diffImage.rows*2)/3; ++j)
        for(int i=diffImage.cols/3; i<(diffImage.cols*2)/3; ++i) {
            cv::Vec3b pix = diffImage.at<cv::Vec3b>(j,i);
            dist = (pix[0]*pix[0] + pix[1]*pix[1] + pix[2]*pix[2]);
            if(dist>threshold) {
               pixdif ++;
          }
        }
    }
    return 1;
}

std::vector<std::string> camera::grab_images(){
   
    printf("scanning...\n");


    std::cout << "can write to this" ;
    std::vector<std::string> filenames; 
    char cwd[PATH_MAX];
    getcwd(cwd,sizeof(cwd));
    strcat(cwd, "/images");
    mkdir(cwd,0700);
    long int count = 0;
    char strname[128] = {NULL};
    char index[2] = {'\0'};


    printf(" \n initialization complete caturing images \n");

    for(int i = 0; i < 3 ; i++) {
        cv::Mat frame;
        cv::Mat frame_pre_processed;

        int LEN = 125;
        double THETA = 0;
        int snr = 700;
        std::vector<uchar> img_buff;
 
        std::time_t t1 = std::time(0);
        std::tm* now = std::localtime(&t1);

        char time_buff[50];
        strftime(time_buff, sizeof(time_buff), "%Y-%m-%d%H%M%S", now);

        this->video_capture >> frame;
        std::vector<int> compression_params;
        compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);
        index[0] = 48 + i;
        strcpy(strname,cwd);
        strcat(strname,"/plate_");
        strcat(strname,index);
        strcat(strname ,time_buff);
        strcat(strname, ".png");
        // imencode(".png",frame,img_buff);

        //this is compression info and such we still need this to save it locally
        //imwrite(strname,frame,compression_params);
        printf(" \n adding file names to strings to array \n"); 

        //printf("created file: %s \n", strname);
        filenames.push_back(std::string(strname));

        std::cout << "adding frame to vector";

        printf("\n adding frame to vector \n");
        images.push_back(frame); 

        printf("\n images have been added to frame  \n");

        if( frame.empty() ) break; // end of video stream
       // imshow("TTCore v.1:)", frame);
        if( cv::waitKey(10) == 27 ) break; // stop capturing by pressing ESC
    }

    return filenames;

}

std::vector<cv::Mat>  camera::get_saved_images(){
    return this->images;

}

camera::camera(int sensitivity, int index, int camera_index){
    this->sensitivity=sensitivity;
    this->index = index;
    this->camera_index = index;
    this->video_capture.open(camera_index);
}

