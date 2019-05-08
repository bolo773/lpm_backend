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
    int analyze_plates();
    int upload_file(std::string filename);
    monitor_instance();
    monitor_instance(int camera_index, int livedb,std::vector<std::string> imp_veh, sql::Connection *con, sqlite3 * backup_db);
     
};
