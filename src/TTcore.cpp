#include "TTcore.hpp"
#include <sys/ioctl.h>
#include <termios.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <error.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

static int callback (void * res,int argc, char ** argv, char **azColName){
    using json = nlohmann::json;

    static int j = 0;

    static nlohmann::json tags_json;
    int i;
    for(i=0;i<argc;i++){

        tags_json[j][azColName[i]] = argv[i] ? argv[i] : "NULL";
    }
    std::string fina = tags_json.dump();
    *(std::string *)res = fina;

    j++;
    return 0;

}

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

std::string TTcore::jsonify_local(){

    using json = nlohmann::json;
    char * zErrMsg = 0;
    json tag;
    int j = 0;
    std::string fina;
    sqlite3_exec(backup_db,"select * from raw_backup inner join image_backup on pid=id",callback,(void*)&fina,&zErrMsg);
    std::ofstream file_out;
    file_out.open("images/backup_data.json");
    
    file_out << fina;
    file_out.close();


    sqlite3_stmt * stmt_backup;
    sqlite3_prepare( this->backup_db, "delete from raw_backup;", -1, &stmt_backup, NULL );
    sqlite3_step(stmt_backup);
    sqlite3_prepare( this->backup_db, "delete from image_backup;", -1, &stmt_backup, NULL );
    sqlite3_step(stmt_backup);
    sqlite3_finalize(stmt_backup);
   
    printf("%s\n",fina.c_str()); 
    return fina;
 

}

int TTcore::interpreter(std::string input ){
    std::istringstream input_stream(input);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{input_stream},
    std::istream_iterator<std::string>{}};
   

   for(int i = 0; i < tokens.size(); i++){

      if(tokens[i] == "help"){

          std::cout << "help_printed \n";
          return 1;


      } else if(tokens[i] == "expUSB" ){

          jsonify_local();
          system("expUSB");
      }

   }

}

int TTcore::serial_monitor(){


    int fd,n,i;
    char buf[64] = "";
    struct termios toptions;
    fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
    printf("fd opened as: ");
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

    if(n < 0) printf("error: %s", strerror(errno));
    buf[n] = 0;
    printf("\nserial monitor initialized \n");
    while(1){

        n = read(fd,buf,64);
        buf[n] = 0;
        if(n > 0){
            std::string input(buf);
            printf("\n using this command: %s !!!!!!!!!!!!!! string size: %d \n",input.c_str(), n);
            this->interpreter("expUSB");

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
     

