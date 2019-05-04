#include "camera.hpp"
#include "alpr.h"

class monitor_instance {
    
    std::vector<std::string> imp_veh;

    camera * camera_device;
    char cwd[PATH_MAX];
    int livedb;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::Connection *con;   
 
    public:

    int monitor();
    int analyze_plates();
    monitor_instance(int camera_index, int livedb,std::vector<std::string> imp_veh, sql::Connection *con, sqlite3 * backup_db);
     
};
