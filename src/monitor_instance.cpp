#include "monitor_instance.hpp"


monitor_instance::monitor_instance(){

this-> camera_device = NULL;
this->livedb = 0;
this->con = NULL;
this->backup_db = NULL;

}


static size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    curl_off_t nread;
    size_t retcode = fread(ptr, size, nmemb, stream);
  
    nread = (curl_off_t)retcode;
  
    fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);
    return retcode;
}

int monitor_instance::upload_data_live(std::string plate_number, bool flagged, std::string accuracy){
        
        std::string sql_q("");
        time_t t = time(0);
        tm* now = localtime(&t);

        char buff[50];
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", now);

        sql_q = "";
        sql_q.append("insert into raw_data (tag_number,t_stamp, flagged, accuracy) values ('");
        sql_q.append(plate_number);
        sql_q.append("', ");
        sql_q.append("'");

        std::ostringstream date_stream(buff);

        sql_q.append(date_stream.str());

        if (flagged)
        sql_q.append("', TRUE,  ");
        else sql_q.append("', FALSE, ");
        sql_q.append(accuracy);
        sql_q.append(");");
        std::cout << sql_q << '\n';
        flagged = 0;
        try{
            this->stmt = this->con->createStatement();
            this->res = this->stmt->executeQuery(sql_q.c_str());
        }

        catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }

}



int monitor_instance::upload_data_backup(std::string plate_number, bool flagged, std::string accuracy){
       
        std::string sql_q("");
        time_t t = time(0);
        tm* now = localtime(&t);
        char buff[50];
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", now);

        sql_q = "";
        sql_q.append("insert into raw_backup (tag_number, flagged, timestamp, confidence) values ('");
       //actual tag number
        sql_q.append(plate_number);
        sql_q.append("', ");

        //std::ostringstream date_stream(buff);
        //sql_q.append(date_stream.str());
        
        //is it flagged?
        if (flagged)
        sql_q.append(" 'TRUE',  '");
        else sql_q.append(" 'FALSE', '");

        std::ostringstream date_stream(buff);
        sql_q.append(date_stream.str());
        sql_q.append("',");

        //the confidence
        sql_q.append(accuracy);
        sql_q.append(");");

        std::cout  <<"-------- query: " << sql_q << "\n";
        printf("storing from backup \n");
        sqlite3_stmt * stmt_backup;
        sqlite3_prepare( this->backup_db, sql_q.c_str(), -1, &stmt_backup, NULL );
        sqlite3_step( stmt_backup );
        sqlite3_finalize(stmt_backup);
        

}

int monitor_instance::store_file_to_backup(std::string filename){
    std::ifstream f1 ("images/" + filename,std::fstream::binary);
    std::ofstream f2 ("image_backup/" + filename, std::fstream::trunc | std::fstream::binary);
    f2 << f1.rdbuf();

}

int monitor_instance::upload_file(std::string filename){

    CURL *curl;
    CURLcode res;
    FILE *hd_src;
    struct stat file_info;
    curl_off_t fsize;
    struct curl_slist *headerlist = NULL;
    std::string command = "RNFR plate_generic.png";
    std::string command2 = "RNTO " + filename;

    hd_src = fopen((std::string("images/") + filename).c_str() ,"rb");
   
     curl = curl_easy_init();
  
    if(curl) {
        headerlist = curl_slist_append(headerlist, command.c_str());
        headerlist = curl_slist_append(headerlist, command2.c_str());

        printf("curl is active\n");
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        curl_easy_setopt(curl, CURLOPT_URL, "ftp://184.173.179.109/plate_generic.png");
        curl_easy_setopt(curl, CURLOPT_USERNAME, "ttcore");
        curl_easy_setopt(curl, CURLOPT_PASSWORD , "infiniti");

        curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
        curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

       res =  curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;

}

monitor_instance::monitor_instance(int camera_index, int livedb, std::vector<std::string> imp_veh, sql::Connection * con, sqlite3 * backup_db){

    getcwd(this->cwd,sizeof(cwd));
    strcat(this->cwd, "/images"); 
    printf("using working directory %s \n", this->cwd);
    this-> camera_device = new camera(0,0,camera_index);
    //this-> camera_device = new camera(0,0,camera_index);
    this -> livedb = livedb;
    this->imp_veh = imp_veh;
    this->con = con;
    this->backup_db = backup_db;
}

int monitor_instance::monitor(){
    while(1){
        camera_device->monitor();
        camera_device->grab_images();
        analyze_plates();
    }
}

int monitor_instance::analyze_plates(){

    std::string sql_q("");
    alpr::Alpr openalpr("us","/etc/openalpr/openalpr.conf");
    openalpr.setTopN(1);

    std::vector<alpr::AlprPlate> detected_plates;
    std::vector<std::string> image_names;
    openalpr.setDefaultRegion("md");
    int flagged = 0;
    if(openalpr.isLoaded() == false){
            std::cerr << "Error loading openALPR" << std::endl;
            return 1;
    }

    char path[100] = {NULL};

    DIR* images_folder = opendir(this->cwd);
    struct dirent * dp;

    while ((dp = readdir(images_folder)) != NULL) {
        strcpy(path,this->cwd);
        strcat(path,"/");
        strcat(path,dp->d_name);
        //printf("found plate %s \n",dp->d_name );
        if (path[strlen(path) -1] == '.') continue;

        image_names.push_back(std::string(dp->d_name));
        alpr::AlprResults results = openalpr.recognize(path);
        if(results.plates.size() <= 0) continue;
            for (int i = 0; i < results.plates.size();i++){
                alpr::AlprPlateResult plate = results.plates[i];
                std::cout << "plate" << i << ":" 
                << plate.topNPlates.size() << 
                "result " << std::endl;

                    for (int k = 0; k < plate.topNPlates.size(); k++){

                         detected_plates.push_back(plate.topNPlates[k]);
                         std::cout << "   : " 
                         << plate.topNPlates[k].characters 
                         << "t\ confidence: " << 
                         plate.topNPlates[k].overall_confidence;
                         std::cout << " pattern match: " 
                         << plate.topNPlates[k].matches_template << std::endl;

                         std::time_t t = std::time(0);
                         std::tm* now = std::localtime(&t);
                         std::cout << (now->tm_year +1900)
                         << (now->tm_mon + 1) << '-'
                         << now->tm_mday
                         << "\n";

                    }

            }

    }

    if (detected_plates.size() > 0 ) {
        alpr::AlprPlate best_fit = detected_plates[0];

        for(int i = 0; i < detected_plates.size(); i++  ){
            if (best_fit.overall_confidence < detected_plates[i].overall_confidence){
                    best_fit = detected_plates[i];

                }

        }
//               int flagged = 0;
               for (int j =0; j < imp_veh.size(); j++) {

                         if(imp_veh[j] == best_fit.characters) {
                         printf("\n\n\n\n\n FLAGGED VEHICLE!!!!!!!!!!!!\n\n\n\n\n");
                         flagged = 1;
                         }

                     }
 
        if (livedb == 1){
            std::cout << "best fit is :" << best_fit.characters << std::endl;
            upload_data_live(best_fit.characters, flagged, std::to_string(best_fit.overall_confidence));
        //time_t now;
        //closedir(images_folder);
        char imageloc[100] = {NULL};

        sql_q = "";
        sql_q = "select max(id) from raw_data;";
   
        try{
            this->stmt = this->con->createStatement();
            this->res = this->stmt->executeQuery(sql_q.c_str());
        }
        catch(sql::SQLException &e){
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
            
        }
         res->next();
         int id =  res->getInt(1);

        for (int l = 0; l < image_names.size(); l++){

        sql_q = "";
        sql_q.append("insert into images (fname,cid) values (");
        sql_q.append((std::string("'//184.173.179.109/") + image_names[l] +"'," + std::to_string(id) + ")").c_str()); 

         try{
            this->stmt = this->con->createStatement();
            this->res = this->stmt->executeQuery(sql_q.c_str());
        }
        catch(sql::SQLException &e){
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;

        }
    }

            //store files
    printf("uploading files\n");
        for (int l = 0; l < image_names.size(); l++){
            upload_file(image_names[l]);
        }

     } else {

        upload_data_backup(best_fit.characters, flagged,std::to_string(best_fit.overall_confidence));
        for (int l = 0; l < image_names.size(); l++){
            store_file_to_backup(image_names[l]);
        }

       

        /*
        time_t t = time(0);
        tm* now = localtime(&t);
        char buff[50];
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", now);

        sql_q = "";
        sql_q.append("insert into raw_backup (tag_number, flagged, timestamp, confidence) values ('");
       //actual tag number
        sql_q.append(best_fit.characters);
        sql_q.append("', ");
        sql_q.append("'");

        //std::ostringstream date_stream(buff);
        //sql_q.append(date_stream.str());
        
        //is it flagged?
        if (flagged)
        sql_q.append("', TRUE,  ");
        else sql_q.append("', FALSE, ");

        std::ostringstream date_stream(buff);
        sql_q.append(date_stream.str());

        //the confidence
        sql_q.append(std::to_string(best_fit.overall_confidence));
        sql_q.append(");");


        sqlite3_stmt * stmt_backup;
        sqlite3_prepare( db, sql_q.c_str(), -1, &stmt_backup, NULL );
        sqlite3_step( stmt_backup );
        sqlite3_finalize(stmt);



        //removing images
        remove("images/*");
        char imageloc[100] = {NULL};
        */
        }
    }
        closedir(images_folder);
        char imageloc[100] = {NULL};
        for (int l = 0; l < image_names.size(); l++){
            strcpy(imageloc,this->cwd);
            strcat(imageloc, "/");
            strcat(imageloc, image_names[l].c_str());
            remove(imageloc);
            printf("removed image\n");

    }
}
