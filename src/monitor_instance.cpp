#include "monitor_instance.hpp"


monitor_instance::monitor_instance(){

this-> camera_device = NULL;
this->livedb = 0;
this->con = NULL;
this->backup_db = NULL;

}


monitor_instance::monitor_instance(int camera_index, int livedb, std::vector<std::string> imp_veh, sql::Connection * con, sqlite3 * backup_db){

    getcwd(this->cwd,sizeof(cwd));
    strcat(this->cwd, "/images"); 
    printf("using working directory %s \n", this->cwd);
    this-> camera_device = new camera(0,0,camera_index);
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
    

        if (livedb == 1){
            std::cout << "best fit is :" << best_fit.characters << std::endl;
            int flagged = 0;
               for (int j =0; j < imp_veh.size(); j++) {

                         if(imp_veh[j] == best_fit.characters) printf("\n\n\n\n\n FLAGGED VEHICLE!!!!!!!!!!!!\n\n\n\n\n");
                         flagged = 1;

                     }

        time_t now;
        //inserting into the database 
        struct tm *local = localtime(&now);

        int hours = local->tm_hour;              // get hours since midnight (0-23)
        int minutes = local->tm_min;             // get minutes passed after the hour (0-59)
        int seconds = local->tm_sec;             // get seconds passed after minute (0-59)

        int day = local->tm_mday;                // get day of month (1 to 31)
        int month = local->tm_mon + 1;           // get month of year (0 to 11)
        int year = local->tm_year + 1900;        // get year since 1900

        char buff[50];
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", local);

        sql_q = "";
        sql_q.append("insert into raw_data (tag_number,t_stamp, flagged, accuracy) values ('");
        sql_q.append(best_fit.characters);
        sql_q.append("', ");
        sql_q.append("'");

        std::ostringstream date_stream(buff);

        sql_q.append(date_stream.str());

        if (flagged)
        sql_q.append("', TRUE,  ");
        else sql_q.append("', FALSE, ");
        sql_q.append(std::to_string(best_fit.overall_confidence));
        sql_q.append(");");

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
        //removing images
        remove("images/*");
        closedir(images_folder);
        char imageloc[100] = {NULL};
        for (int l = 0; l < image_names.size(); l++){
            strcpy(imageloc,this->cwd);
            strcat(imageloc, "/");
            strcat(imageloc, image_names[l].c_str());
            remove(imageloc);
        }

    } 
}
}
