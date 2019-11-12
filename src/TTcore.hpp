#include "monitor_instance.hpp"
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

class TTcore{

    sql::Driver * driver;
    sql::Connection * con;
    sql::Statement * stmt;
    sql::ResultSet * res;
    sqlite3 *backup_db;
    int camera_index;
    std::vector<std::string> imp_veh;

    monitor_instance main_thread;

public:

    TTcore(int);
    int init();
    int start();
    int interpreter(std::string input);
    std::string jsonify_local();
};
