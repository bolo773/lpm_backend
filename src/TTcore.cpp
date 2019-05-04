#include "TTcore.hpp"

TTcore::TTcore(){

std::string sql_q = "";


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


    if( rc ) {
      fprintf(stderr, "Warning! can't open backup database: %s\n", sqlite3_errmsg(backup_db));
    } else {
      fprintf(stdout, "Opened backup database successfully\n");
    }

    this->main_thread = monitor(1,1, ,this->con, this->backup_db);

}



