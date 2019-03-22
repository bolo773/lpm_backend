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
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;


int wait_for_motion(VideoCapture* cap){
       Mat frame1, frame2;

       float dist;
       clock_t ticks1, ticks2;
       int pixdif = 0;

       float threshold = 30.0f;
       cv::Mat diffImage;

       while(1){

       *cap >> frame1;

	ticks1=clock();
       	ticks2=ticks1;

	while((ticks2/CLOCKS_PER_SEC-ticks1/CLOCKS_PER_SEC)<.5) ticks2=clock();

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
          
	   if(pixdif > 1000) return 1;
     }
   }
}



int main(int argc, char *argv[]){
	
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
       if(!cap.open(0))
       return 0;

      //start loop
      while(1) {
      wait_for_motion(&cap);


       for(int i = 0; i < 3 ; i++)
           {
             Mat frame;
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

		std::cout << path << std::endl;


		
		alpr::AlprResults results = openalpr.recognize(path);
                if(results.plates.size() <= 0) continue;
		for (int i = 0; i < results.plates.size();i++){
			alpr::AlprPlateResult plate = results.plates[i];
			std::cout << "plate" << i << ":" << plate.topNPlates.size() << "result " << std::endl;

			for (int k = 0; k < plate.topNPlates.size(); k++){
		
				detected_plates.push_back(plate.topNPlates[k]);
				std::cout << "   : " << plate.topNPlates[k].characters << "t\ confidence: " << plate.topNPlates[k].overall_confidence;
				std::cout << " pattern match: " << plate.topNPlates[k].matches_template << std::endl;
			}
	
		}
	}

	alpr::AlprPlate best_fit = detected_plates[0];

	if (detected_plates.size() <= 0 ) continue;
	for(int i = 0; i < detected_plates.size(); i++  ){
		if (best_fit.overall_confidence < detected_plates[i].overall_confidence){
			best_fit = detected_plates[i];
		}
	
	}
	std::cout << "best fit is :" << best_fit.characters << std::endl;
        remove("images/frame1.png");
        remove("images/frame2.png");
        remove("images/frame3.png");	

      }
//	closedir(images_folder);
}
