#include "monitor_instance.hpp"

monitor_instance::monitor_instance(int camera_index, int livedb){

    getcwd(this->cwd,sizeof(cwd));
    strcat(this->cwd, "/images"); 
    printf("using working directory %s \n", this->cwd);
    this-> camera_device = new camera(camera_index,0,0);
    this -> livedb = livedb;
    
}

int monitor_instance::monitor(){
    while(1){
        camera_device->monitor();
        camera_device->grab_images();
        analyze_plates();
    }
}

int monitor_instance::analyze_plates(){


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
        printf("found plate %s \n",dp->d_name );
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

        }
    }
   
    printf("right before delete \n");
    char imageloc[100] = {"\0"};
    for (int l = 0; l < image_names.size(); l++){
        strcpy(imageloc,this->cwd);
        strcpy(imageloc, "/images/");
        printf(" image location: %s\n",image_names[l].c_str());
        strcat(imageloc, image_names[l].c_str());
        strcat(imageloc,'\0');

        printf("%s",imageloc); 
     //   remove(imageloc);
    }

} 
