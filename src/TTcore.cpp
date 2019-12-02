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

int test_conn(){

    FILE *output;
    if(!(output = popen("/sbin/route -n | grep -c '^0\\.0\\.0\\.0'","r"))){
        return 0;
    }
    unsigned int i;
    fscanf(output,"%u",&i);
    
    pclose(output);
    if(i==0)
           return 0;
    else if(i==1)
        return 1;


}


static size_t write_callback( char * contents, size_t size, size_t nmemb, void * userp){

    ((std::string*) userp) -> append((char * )contents, size * nmemb);
    return size * nmemb;

}

std::vector<std::string> get_impveh()
{

    using json = nlohmann::json;
    std::vector<std::string> final_results;
    json tags;

    std::string readbuffer;


    CURL *curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    /* Check for errors */
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n",curl_easy_strerror(res));
        return final_results;
  }

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://104.154.27.143/imp_veh.php");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readbuffer);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) 
        fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    
    printf( "%s\n", readbuffer.c_str() );
    tags = json::parse(readbuffer);
    //for(json::iterator it = tags.begin() ; it != tags.end() ; ++it ){
    for (int it = 0; it < tags.size(); it ++){    
    printf( "got important tag : %s\n",tags[it]["tag_number"].get<std::string>().c_str());

    final_results.push_back(tags[it]["tag_number"].get<std::string>().c_str());
    }

    return final_results;
}

                              
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
    int livedb = test_conn();
    std::string sql_q = "";
    sqlite3 *backup_db;
    printf("the test_conn worked\n"); 

   this -> imp_veh = std::vector<std::string>();
   // sql_q.append("select * from flagged_vehicles");
   // this->driver = get_driver_instance();
   // try{
   //     this->con = driver->connect("tcp://184.173.179.108:3306", "bolo7_tag_user", "infiniti");
   //     this->con->setSchema("bolo773_ttcore");
   //     this->stmt = con->createStatement();
   //     this->res = stmt->executeQuery(sql_q.c_str());
   // }

   // catch (sql::SQLException &e) {
   // std::cout << "# ERR: SQLException in " << __FILE__;
   // std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
   // std::cout << "# ERR: " << e.what();
   // std::cout << " (MySQL error code: " << e.getErrorCode();
   // std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
   // livedb = 0;

   // }

    int rc = sqlite3_open("backup.db", &backup_db);
    this->backup_db = backup_db;

    if( rc ) {
      fprintf(stderr, "Warning! can't open backup database: %s\n", sqlite3_errmsg(backup_db));
    } else {
      fprintf(stdout, "Opened backup database successfully\n");
    }

    //populate the important vehicles list
    if (livedb == 1){
        //while (res->next()) {
        //std::cout << "got important tag" << res->getString(2);
        //imp_veh.push_back(res->getString(2));
       
     try{
         this->imp_veh = get_impveh(); 
        } catch(...) {
            printf("Cannot get live db info!\n");
            livedb = 0;
        }

    //} 

    //create the main monitor thread

    } else printf(" live db is not online %d \n",livedb);

    this->main_thread = monitor_instance(camera_index ,livedb ,this->imp_veh,this->con, this->backup_db);
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
     

