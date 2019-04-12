#include "alpr.h"
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sstream>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
//#include "opencv2/videoio/videoio.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgcodecs.hpp"
#include <ctime>
#include  <thread>
#include <pthread.h>
//#include <sqlite3.h>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace cv;

std::fstream *  init(){

    //init log file

    std::cout << "TTCore Version .01 \n";

    std::cout << "Initializing log File";
    std::fstream * logfile = new std::fstream;
    logfile->open ("TTCore.log");
    *logfile << "Session starts\n";

    return logfile;

}

int interpreter(std::string input ){

    std::istringstream input_stream(input);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{input_stream},
    std::istream_iterator<std::string>{}};


   for(int i = 0; i < tokens.size(); i++){
   
      if(tokens[i] == "help"){
      
          std::cout << "help_printed \n";
	  return 1;

          
      } else if(tokens[i] == "setSensitivity" ){
           int  sensitivity = std::stoi(tokens[i+1]);

            

      }
   
   } 


}

/*
void calcPSF(Mat& outputImg, Size filterSize, int len, double theta)
{
    Mat h(filterSize, CV_32F, Scalar(0));
    Point point(filterSize.width / 2, filterSize.height / 2);
    ellipse(h, point, Size(0, cvRound(float(len) / 2.0)), 90.0 - theta, 0, 360, Scalar(255), FILLED);
    Scalar summa = sum(h);
    outputImg = h / summa[0];
}
void fftshift(const Mat& inputImg, Mat& outputImg)
{
    outputImg = inputImg.clone();
    int cx = outputImg.cols / 2;
    int cy = outputImg.rows / 2;
    Mat q0(outputImg, Rect(0, 0, cx, cy));
    Mat q1(outputImg, Rect(cx, 0, cx, cy));
    Mat q2(outputImg, Rect(0, cy, cx, cy));
    Mat q3(outputImg, Rect(cx, cy, cx, cy));
    Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
}

void filter2DFreq(const Mat& inputImg, Mat& outputImg, const Mat& H)
{
    Mat planes[2] = { Mat_<float>(inputImg.clone()), Mat::zeros(inputImg.size(), CV_32F) };
    Mat complexI;
    merge(planes, 2, complexI);
    dft(complexI, complexI, DFT_SCALE);
    Mat planesH[2] = { Mat_<float>(H.clone()), Mat::zeros(H.size(), CV_32F) };
    Mat complexH;
    merge(planesH, 2, complexH);
    Mat complexIH;
    mulSpectrums(complexI, complexH, complexIH, 0);
    idft(complexIH, complexIH);
    split(complexIH, planes);
    outputImg = planes[0];
}

void calcWnrFilter(const Mat& input_h_PSF, Mat& output_G, double nsr)
{
    Mat h_PSF_shifted;
    fftshift(input_h_PSF, h_PSF_shifted);
    Mat planes[2] = { Mat_<float>(h_PSF_shifted.clone()), Mat::zeros(h_PSF_shifted.size(), CV_32F) };
    Mat complexI;
    merge(planes, 2, complexI);
    dft(complexI, complexI);
    split(complexI, planes);
    Mat denom;
    pow(abs(planes[0]), 2, denom);
    denom += nsr;
    divide(planes[0], denom, output_G);
}

void edgetaper(const Mat& inputImg, Mat& outputImg, double gamma = 5.0, double beta = 0.2)
{
    int Nx = inputImg.cols;
    int Ny = inputImg.rows;
    Mat w1(1, Nx, CV_32F, Scalar(0));
    Mat w2(Ny, 1, CV_32F, Scalar(0));
    float* p1 = w1.ptr<float>(0);
    float* p2 = w2.ptr<float>(0);
    float dx = float(2.0 * CV_PI / Nx);
    float x = float(-CV_PI);
    for (int i = 0; i < Nx; i++)
    {
        p1[i] = float(0.5 * (tanh((x + gamma / 2) / beta) - tanh((x - gamma / 2) / beta)));
        x += dx;
    }
    float dy = float(2.0 * CV_PI / Ny);
    float y = float(-CV_PI);
    for (int i = 0; i < Ny; i++)
    {
        p2[i] = float(0.5 * (tanh((y + gamma / 2) / beta) - tanh((y - gamma / 2) / beta)));
        y += dy;
    }
    Mat w = w2 * w1;
    multiply(inputImg, w, outputImg);
}



*/

int wait_for_motion(VideoCapture* cap){


       Mat frame1, frame2;

       float dist;
       clock_t ticks1, ticks2;

       float threshold = 50.0f;
       cv::Mat diffImage;

       while(1){

       *cap >> frame1;

	ticks1=clock();
       	ticks2=ticks1;

	while((ticks2/CLOCKS_PER_SEC-ticks1/CLOCKS_PER_SEC)<.002) ticks2=clock();

       int pixdif = 0;
       *cap >> frame2;

	   cv::absdiff(frame1, frame2, diffImage);

	   cv::Mat foregroundMask = cv::Mat::zeros(diffImage.rows, diffImage.cols, CV_8UC1);

          for(int j=0; j<diffImage.rows; ++j)
          for(int i=0; i<diffImage.cols; ++i)
	   {
	     cv::Vec3b pix = diffImage.at<cv::Vec3b>(j,i);
             dist = (pix[0]*pix[0] + pix[1]*pix[1] + pix[2]*pix[2]);

	   if(dist>threshold)
           {
		   pixdif ++;
	   }
          
     }


	   if(pixdif > 65000) return 1;
   }
}

int monitor(){

       sql::Driver * driver;
       sql::Connection *con;
       sql::Statement *stmt;
       sql::ResultSet *res;

      driver = get_driver_instance();

       con = driver->connect("tcp://127.0.0.1:3306", "camera", "infiniti");
       con->setSchema("tagTracker");

	
       char cwd[PATH_MAX];
       getcwd(cwd,sizeof(cwd));

       strcat(cwd, "/images");
       mkdir(cwd,0700);

       long int count = 0;
       VideoCapture cap;
       char strname[128] = {NULL};
       char index[2];
       // open the default camera, use something different from 0 otherwise;
       // Check VideoCapture documentation.
       if(!cap.open(1))
       exit(0);

      //start loop
      while(1) {
      wait_for_motion(&cap);


       for(int i = 0; i < 3 ; i++)
           {
             Mat frame;
	     Mat frame_pre_processed;
              
             int LEN = 125;
             double THETA = 0;
	     int snr = 700;


	     // it needs to process even image only
//             Rect roi = Rect(0, 0, frame.cols & -2, frame.rows & -2);
             //Hw calculation (start)
  //           Mat Hw, h;
      //       calcPSF(h, roi.size(), LEN, THETA);
    //         calcWnrFilter(h, Hw, 1.0 / double(snr));
              //Hw calculation (stop)
     //        frame.convertTo(frame, CV_32F);
       //      edgetaper(frame, frame);
    	     // filtering (start)
    	 //   filter2DFreq(frame(roi), frame_pre_processed, Hw);
    	    // filtering (stop)
   // 	   frame_pre_processed.convertTo(frame_pre_processed, CV_8U);
   //	   normalize(frame_pre_processed, frame_pre_processed, 0, 255, NORM_MINMAX);


             cap >> frame;
             std::vector<int> compression_params;
             compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
             compression_params.push_back(9);
             index[0] = 48 + i;
             strcpy(strname,cwd);
	     strcat(strname,"/frame");
             strcat(strname ,index);
             strcat(strname, ".png");
             imwrite(strname,frame,compression_params);
             

             if( frame.empty() ) break; // end of video stream
	  
             imshow("this is you, smile! :)", frame);
             if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
         }

	alpr::Alpr openalpr("us","/etc/openalpr/openalpr.conf");
	openalpr.setTopN(1);

	std::vector<alpr::AlprPlate> detected_plates;
	
	openalpr.setDefaultRegion("md");

	if(openalpr.isLoaded() == false){
		std::cerr << "Error loading openALPR" << std::endl;
		return 1;
	}
	
	char path[100] = {NULL};

	DIR* images_folder = opendir(cwd);
	struct dirent * dp;

	while ((dp = readdir(images_folder)) != NULL) {
		strcpy(path,cwd);
		strcat(path,"/");
		strcat(path,dp->d_name);
		
		if (path[strlen(path) -1] == '.'){
		continue;
		
		}

		//std::cout << path << std::endl;


		
		alpr::AlprResults results = openalpr.recognize(path);
                if(results.plates.size() <= 0) continue;
		for (int i = 0; i < results.plates.size();i++){
			alpr::AlprPlateResult plate = results.plates[i];
			std::cout << "plate" << i << ":" << plate.topNPlates.size() << "result " << std::endl;

			for (int k = 0; k < plate.topNPlates.size(); k++){
		
				detected_plates.push_back(plate.topNPlates[k]);
				std::cout << "   : " << plate.topNPlates[k].characters << "t\ confidence: " << plate.topNPlates[k].overall_confidence;
				std::cout << " pattern match: " << plate.topNPlates[k].matches_template << std::endl;

				std::time_t t = std::time(0);
				std::tm* now = std::localtime(&t);
				std::cout << (now->tm_year +1900)
				<< (now->tm_mon + 1) << '-'
				<< now->tm_mday
				<< "\n";

			}
	
		}
	}

        
	if (detected_plates.size() <= 0 ) continue;
	alpr::AlprPlate best_fit = detected_plates[0];

	for(int i = 0; i < detected_plates.size(); i++  ){
		if (best_fit.overall_confidence < detected_plates[i].overall_confidence){
			best_fit = detected_plates[i];
		}
	
	}
	std::cout << "best fit is :" << best_fit.characters << std::endl;
        
      try{
      stmt = con->createStatement();
       res = stmt->executeQuery("insert into raw_data(tStamp,fileSize) values ('990319', 10)");
        }

         catch (sql::SQLException &e) {
 std::cout << "# ERR: SQLException in " << __FILE__;
  std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
 std::cout << "# ERR: " << e.what();
  std::cout << " (MySQL error code: " << e.getErrorCode();
  std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
}



        remove("images/frame0.png");
        remove("images/frame1.png");
        remove("images/frame2.png");	

      }
//	closedir(images_folder);
}


int main(int argc, char *argv[]){
	std::thread thread_obj(monitor);
	std::string input;
	std::fstream * logptr = init();
	while(1){
        std::getline (std::cin,input);
	interpreter(input);
	std::cout << "TTCore:>>>" ;


	}

}
