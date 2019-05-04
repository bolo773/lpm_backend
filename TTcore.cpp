#include "alpr.h"
#include <stdio.h>
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
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <ctime>
#include <thread>
#include <pthread.h>
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
#include <sqlite3.h>

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

int wait_for_motion(VideoCapture* cap){

       Mat frame1, frame2;
       float dist;
       clock_t ticks1, ticks2;
       float threshold = 50.0f;
       cv::Mat diffImage;
       int pixdif = 0;

       while(1){

	ticks1=clock();

       	ticks2=ticks1;

        //wait between every initial capture
        while((ticks2/CLOCKS_PER_SEC-ticks1/CLOCKS_PER_SEC)<.1) ticks2=clock();

        *cap >> frame1;




	while((ticks2/CLOCKS_PER_SEC-ticks1/CLOCKS_PER_SEC)<.002) ticks2=clock();

       pixdif = 0;
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


	   if(pixdif > 35000) return 1;
   }
}

int monitor(){

    sql::Driver * driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
       
    std::vector<std::string> imp_veh;

    driver = get_driver_instance();


    std::string sql_q = "";
  
    int livedb = 1;    

    sql_q.append("select * from flagged_vehicles");
    

    try{
        
        con = driver->connect("tcp://184.173.179.108:3306", "bolo7_tag_user", "infiniti");
        con->setSchema("bolo773_ttcore");
        stmt = con->createStatement();
        res = stmt->executeQuery(sql_q.c_str());
    }

    catch (sql::SQLException &e) {
    std::cout << "# ERR: SQLException in " << __FILE__;
    std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
    std::cout << "# ERR: " << e.what();
    std::cout << " (MySQL error code: " << e.getErrorCode();
    std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    livedb = 0;

    
    } 

    sqlite3 *backup_db;
    
    int rc = sqlite3_open("backup.db", &backup_db);
 

    if( rc ) {
      fprintf(stderr, "Warning! can't open backup database: %s\n", sqlite3_errmsg(backup_db));
    } else {
      fprintf(stdout, "Opened backup database successfully\n");
   }



    std::string s_sql="";

    if (livedb == 1){
    while (res->next()) {
    // You can use either numeric offsets...
    std::cout << "got important tag" << res->getString(2); // getInt(1) returns the first column

    //std::string platename(res->getString.c_str());
    imp_veh.push_back(res->getString(2));

    // ... or column names for accessing results.
    // The latter is recommended.
    }
}
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
        for(int i = 0; i < 3 ; i++) {
            Mat frame;
	    Mat frame_pre_processed;
              
            int LEN = 125;
            double THETA = 0;
	    int snr = 700;

            cap >> frame;
            std::vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(9);
            index[0] = 48 + i;
            strcpy(strname,cwd);
	    strcat(strname,"/plate_");
            strcat(strname ,index);
            strcat(strname, ".png");
            imwrite(strname,frame,compression_params);

            if( frame.empty() ) break; // end of video stream
	    imshow("TTCore v.1:)", frame);
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


        if (livedb == 1){
	std::cout << "best fit is :" << best_fit.characters << std::endl;
               int flagged = 0;
               for (int j =0; j < imp_veh.size(); j++) {

                         if(imp_veh[j] == best_fit.characters) printf("\n\n\n\n\n FLAGGED VEHICLE!!!!!!!!!!!!\n\n\n\n\n");
                         flagged = 1;

                     }
        




     	time_t now;
	
	// Obtain current time
	// time() returns the current time of the system as a time_t value
	time(&now);

	// Convert to local time format and print to stdout
	printf("Today is : %s", ctime(&now));

	// localtime converts a time_t value to calendar time and 
	// returns a pointer to a tm structure with its members 
	// filled with the corresponding values

//       std::tm* now = std::localtime(&t);
       struct tm *local = localtime(&now);

       int hours = local->tm_hour;      	// get hours since midnight (0-23)
       int minutes = local->tm_min;     	// get minutes passed after the hour (0-59)
       int seconds = local->tm_sec;     	// get seconds passed after minute (0-59)

       int day = local->tm_mday;        	// get day of month (1 to 31)
       int month = local->tm_mon + 1;   	// get month of year (0 to 11)
       int year = local->tm_year + 1900;	// get year since 1900



      char buff[50];
      strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", local);

      sql_q = "";
      sql_q.append("insert into raw_data (tag_number,t_stamp, flagged, accuracy) values ('");
      sql_q.append(best_fit.characters);
      sql_q.append("', ");
      sql_q.append("'");
      
      std::ostringstream date_stream(buff);


      sql_q.append(date_stream.str());

      if (flagged)
      sql_q.append("', FALSE,  ");
      else sql_q.append("', TRUE, ");
      sql_q.append(std::to_string(best_fit.overall_confidence));
      sql_q.append(");");      

      flagged = 0;


      try{
      stmt = con->createStatement();

       
       res = stmt->executeQuery(sql_q.c_str());
        }

         catch (sql::SQLException &e) {
    std::cout << "# ERR: SQLException in " << __FILE__;
    std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
    std::cout << "# ERR: " << e.what();
    std::cout << " (MySQL error code: " << e.getErrorCode();
    std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }


       // remove("images/frame0.png");
       // remove("images/frame1.png");
        remove("images/*");	

      }
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
