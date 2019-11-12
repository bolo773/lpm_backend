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

class monitor_instance {
    
    std::vector<std::string> imp_veh;

    camera * camera_device;
    char cwd[PATH_MAX];
    int livedb;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::Connection *con;   
    sqlite3 * backup_db; 
    public:

    int monitor();
    std::string upload_file(char *, int);
    int analyze_plates();
    int analyze_plates_offline();
    int recognize_plates();

    std::string insert_image_backup(std::string,std::string);
    std::string upload_data_live(std::string, bool, std::string);
    std::string save_data_backup(std::string, bool, std::string);
    int store_file_to_backup(std::string filename);
    std::string write_to_disk(cv::Mat);
    int start();
    monitor_instance();
    monitor_instance(int camera_index, int livedb,std::vector<std::string> imp_veh, sql::Connection *con, sqlite3 * backup_db);
     
};
