#include "TTcore.hpp"
#include <sys/ioctl.h>
#include <termios.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <error.h>
TTcore::TTcore(int camera_index){
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

    } else printf(" live db is not online %d \n",livedb);

    this->main_thread = monitor_instance(camera_index ,livedb ,imp_veh ,this->con, this->backup_db);
}
/*
std::string jsonify_local(){

   std::string res("returned fine");
   return  res;

}
*/
int TTcore::interpreter(std::string input ){
    std::istringstream input_stream(input);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{input_stream},
    std::istream_iterator<std::string>{}};
   

   for(int i = 0; i < tokens.size(); i++){

      if(tokens[i] == "help"){

          std::cout << "help_printed \n";
          return 1;


      } else if(tokens[i] == "expUSB" ){

          //jsonify_local();
          system("expUSB");
      }

   }

}

int TTcore::serial_monitor(){


    int fd,n,i;
    char buf[64] = "";
    struct termios toptions;
    fd = open("/dev/ttyACMO", O_RDWR | O_NOCTTY);
    printf("fd opened as");
    tcgetattr(fd,&toptions);
    cfsetispeed(&toptions,B9600);
    cfsetospeed(&toptions,B9600);

    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag  &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    toptions.c_cflag |= ICANON;
    tcsetattr(fd,TCSANOW,&toptions);
    write(fd,"0",1);
    n = read(fd,buf,64);
    buf[n] = 0;

    while(1){

        n = read(fd,buf,64);
        buf[n] = 0;
        if(n > 0){
            std::string input(buf);
            this->interpreter(input);
        }
    }
}

int TTcore::start(){

    std::string input;
    this->main_thread.start();
    std::thread serial_mon(&TTcore::serial_monitor,this);
    serial_mon.detach();

    while(1){

    std::getline (std::cin,input);
    this->interpreter(input);
    std::cout << "TTCore:>>>";
    }

}
     

