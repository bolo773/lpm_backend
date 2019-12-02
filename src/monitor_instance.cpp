#include "monitor_instance.hpp"
#include "analyzer.hpp"
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <mutex>


int monitor_instance::data_insertion_loop () {

    while(1){
    if(insert_qeue.size() != 0){

          if(insert_qeue.back().live_db()) insert_qeue.back().analyze_plates();
          else insert_qeue.back().analyze_plates_offline();
          insert_qeue.pop_back();
      } 
    }
   return 1;
}

monitor_instance::monitor_instance(){

    this-> camera_device = NULL;
    this->livedb = 0;
    this->con = NULL;
    this->backup_db = NULL;

}

int monitor_instance::test_conn(){

    FILE *output;
    if(!(output = popen("/sbin/route -n | grep -c '^0\\.0\\.0\\.0'","r"))){
        return 0;
    }
    unsigned int i;
    fscanf(output,"%u",&i);
    if(i==0)
           return 0;
    else if(i==1)
        return 1;

    pclose(output);

}

monitor_instance::monitor_instance(int camera_index, int livedb, std::vector<std::string> imp_veh, sql::Connection * con, sqlite3 * backup_db){

    getcwd(this->cwd,sizeof(cwd));
    strcat(this->cwd, "/images"); 
    printf("\nusing working directory %s \n", this->cwd);
    this-> camera_device = new camera(0,0,camera_index);
    this -> livedb = livedb;
    printf("\n can get past dir init \n");
    this->imp_veh = std::vector<std::string>(imp_veh);
    printf("\n can get past vector\n");
    this->con = con;
    this->backup_db = backup_db;
    this-> insert_qeue = std::vector<analyzer>();
    analyzer::set_impveh(imp_veh);
}

int monitor_instance::monitor(){

    while(1){
        camera_device->monitor();
        camera_device->grab_images();
        std::vector<cv::Mat> plates = camera_device->get_saved_images();
       
       // printf( " current:  %s   \n",imp_veh[0].c_str());
        if(test_conn()){
            printf("\nsystem online\n");
            analyzer result = analyzer(plates,this->backup_db,1, this->imp_veh);
            insert_qeue.push_back(result);    
        } else {
            printf("\nsystem offline\n");
            analyzer result = analyzer(plates,this->backup_db,0, this->imp_veh);
            insert_qeue.push_back(result);
        }
        camera_device->free_images();
       
    }
}

int monitor_instance::start(){

    std::thread mainthread(&monitor_instance::monitor,this);
    mainthread.detach();
    std::thread insertthread(&monitor_instance::data_insertion_loop,this);
    insertthread.detach();
    return 1;
}

