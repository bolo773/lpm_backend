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
#include <veh_entry.hpp>
class insert_queue{
    
    std::vector<std::string> imp_veh;

    camera * camera_device;
    int livedb;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::Connection *con;   
    sqlite3 * backup_db; 
    std::vector<veh_entry> queue;
    public:

    int analyze_plates();
    int upload_file(std::string filename);
    int recognize_plates();
    int upload_data_live(std::string, bool, std::string);
    int upload_data_backup(std::string, bool, std::string);
    int store_file_to_backup(std::string filename);
    int start();
    insert_queue();
    insert_queue(int livedb,std::vector<std::string> imp_veh, sql::Connection * con, sqlite3 * backup_db);
   int add_queue(veh_entry);  
};
