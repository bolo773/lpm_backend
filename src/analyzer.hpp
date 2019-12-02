#include "camera.hpp"
#include "alpr.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <algorithm>
#include <iterator>
#include <sqlite3.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <thread>

#ifndef _analyzer
class  analyzer{
     
    char cwd[PATH_MAX];
    int livedb;
    sqlite3 * backup_db; 
    std::vector <cv::Mat> plates;
    public:

    static std::vector <std::string> imp_veh;   
    static int set_impveh(std::vector<std::string> imp_veh);
    std::string upload_file(char *, int);
    int analyze_plates();
    int analyze_plates_offline();
    int recognize_plates();
 
    std::string insert_image_backup(std::string,std::string);
    std::string upload_data_live(std::string, bool, std::string);
    std::string save_data_backup(std::string, bool, std::string);
    int store_file_to_backup(std::string filename);
    std::string write_to_disk(cv::Mat);
    int analyze();
    int insert();
    analyzer();
    analyzer(std::vector<cv::Mat>, sqlite3 * backup_db,int, std::vector<std::string>);
    int live_db(); 
};

#define _analyzer
#endif
