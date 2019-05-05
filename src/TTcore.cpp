#include "TTcore.hpp"

TTcore::TTcore(){
    int livedb = 1;
    std::string sql_q = "";
    sqlite3 *backup_db;
    

    sql_q.append("select * from flagged_vehicles");
    this->driver = get_driver_instance();
    try{
        this->con = driver->connect("tcp://184.173.179.108:3306", "bolo7_tag_user", "infiniti");
        this->con->setSchema("bolo773_ttcore");
        this->stmt = con->createStatement();
        this->res = stmt->executeQuery(sql_q.c_str());
    }

    catch (sql::SQLException &e) {
    std::cout << "# ERR: SQLException in " << __FILE__;
    std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
    std::cout << "# ERR: " << e.what();
    std::cout << " (MySQL error code: " << e.getErrorCode();
    std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    livedb = 0;

    }

    int rc = sqlite3_open("backup.db", &backup_db);
    this->backup_db = backup_db;

    if( rc ) {
      fprintf(stderr, "Warning! can't open backup database: %s\n", sqlite3_errmsg(backup_db));
    } else {
      fprintf(stdout, "Opened backup database successfully\n");
    }

    //populate the important vehicles list
    if (livedb == 1){
        while (res->next()) {
        std::cout << "got important tag" << res->getString(2);
        imp_veh.push_back(res->getString(2));
    }

    //create the main monitor thread

    }

    this->main_thread = monitor_instance(0,livedb,imp_veh,this->con, this->backup_db);
}

int TTcore::start(){
    this->main_thread.monitor();
    

}
