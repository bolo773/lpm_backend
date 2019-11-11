#include "monitor_instance.hpp"
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

int write_to_disk(cv::Mat plate,int ind){

    char index[2] = {'\0'};
    index[0] = 48 + ind;    
    std::time_t t1 = std::time(0);
    std::tm* now = std::localtime(&t1);

    char cwd[PATH_MAX];
    getcwd(cwd,sizeof(cwd));
    strcat(cwd, "/images");
    char time_buff[50];

    strftime(time_buff, sizeof(time_buff), "%Y-%m-%d%H%M%S", now);

    char strname[128] = {NULL};
    strcpy(strname,cwd);
    strcat(strname,"/plate_");
    strcat(strname,index);
    strcat(strname ,time_buff);
    strcat(strname, ".png");

    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

    imwrite(strname,plate,compression_params);


}


monitor_instance::monitor_instance(){

this-> camera_device = NULL;
this->livedb = 0;
this->con = NULL;
this->backup_db = NULL;

}

/*
static size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    curl_off_t nread;
    size_t retcode = fread(ptr, size, nmemb, stream);
  
    nread = (curl_off_t)retcode;
  
    //fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
    //      " bytes from file\n", nread);

    printf(".");
    return retcode;
}
*/

/* silly test data to POST */
static char data[100] = {'\0'};
struct WriteThis {
  const char *readptr;
  size_t sizeleft;
};

static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *wt = (struct WriteThis *)userp;
  size_t buffer_size = size*nmemb;

  if(wt->sizeleft) {
    /* copy as much as possible from the source to the destination */
    size_t copy_this_much = wt->sizeleft;
    if(copy_this_much > buffer_size)
    copy_this_much = buffer_size;
    memcpy(dest, wt->readptr, copy_this_much);

    wt->readptr += copy_this_much;
    wt->sizeleft -= copy_this_much;
    return copy_this_much; /* we copied this many bytes */
  }

  return 0; /* no more data left to deliver */
}

static size_t write_callback( char * contents, size_t size, size_t nmemb, void * userp){

    ((std::string*) userp) -> append((char * )contents, size * nmemb);
    return size * nmemb;

}

int upload_data(std::string tag_number, std::string confidence, std::string timestamp, std::string flagged)
{

  using json = nlohmann::json;

  json tag;

  tag["number"] = tag_number;
  tag["confidence"] = confidence;
  tag["timestamp"] = timestamp;
  tag["flagged"] = flagged;
  std::string json_data = tag.dump();

  const std::string::size_type size = json_data.size();

  memcpy(data,(const void*)json_data.c_str(),size + 1);

  CURL *curl;
  CURLcode res;

  struct WriteThis wt;

  wt.readptr = data;
  wt.sizeleft = strlen(data);

  /* In windows, this will init the winsock stuff */
  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  /* Check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n",
            curl_easy_strerror(res));
    return 1;
  }

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. */
    curl_easy_setopt(curl, CURLOPT_URL, "http://104.154.27.143/insert.php");

    /* Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    /* pointer to pass to our read function */
    curl_easy_setopt(curl, CURLOPT_READDATA, &wt);

    /* get verbose debug output please */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs,"Content-Type: application/json");
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,hs);

    /* Set the expected POST size. If you want to POST large amounts of data,
       consider CURLOPT_POSTFIELDSIZE_LARGE */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);


    printf("\n  retcode:   %d \n", res);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}







int monitor_instance::upload_data_live(std::string plate_number, bool flagged, std::string accuracy){


        time_t t = time(0);
        tm* now = localtime(&t);

        char buff[50];
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", now);
    
        std::ostringstream date_stream(buff);

        upload_data(plate_number, accuracy, date_stream.str(), "0");
    
    return 1;
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

std::string monitor_instance::upload_file(char * data,int data_size){

  CURL *curl;
  CURLcode res;
  
  std::string readbuffer;
  struct WriteThis wt;

  wt.readptr = data;
  wt.sizeleft = data_size;

  /* In windows, this will init the winsock stuff */
  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  /* Check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n",
            curl_easy_strerror(res));
    return "curl failed no upload";
  }

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. */
    curl_easy_setopt(curl, CURLOPT_URL, "http://104.154.27.143/image_upload.php?tag_id=1");

    /* Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_callback);


    curl_easy_setopt(curl,CURLOPT_WRITEDATA, &readbuffer);

    /* pointer to pass to our read function */
    curl_easy_setopt(curl, CURLOPT_READDATA, &wt);

    /* get verbose debug output please */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  
    

    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs,"Content-Type: application/blob");
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,hs);


    std::string readbuffer;

     /* Set the expected POST size. If you want to POST large amounts of data,
       consider CURLOPT_POSTFIELDSIZE_LARGE */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);



    printf("%s", readbuffer.c_str());


    printf("\n  retcode:   %d \n", res);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return readbuffer;


}


int test_conn(){

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
        printf("\n before grab images  \n");
        camera_device->grab_images();
        printf("\n analyze plates  \n");
        analyze_plates();
    }
}

int monitor_instance::start(){
    std::thread mainthread(&monitor_instance::monitor,this);
    mainthread.detach();

}

int monitor_instance::analyze_plates(){

    std::string sql_q("");
    alpr::Alpr openalpr("us","/etc/openalpr/openalpr.conf");
    openalpr.setTopN(1);

    std::vector< std::string> image_names;

    std::vector<alpr::AlprPlate> detected_plates;
    openalpr.setDefaultRegion("md");
    int flagged = 0;
    if(openalpr.isLoaded() == false){
            std::cerr << "Error loading openALPR" << std::endl;
            return 1;
    }

    char path[100] = {NULL};

    DIR* images_folder = opendir(this->cwd);
    struct dirent * dp;

   

    while (camera_device->get_saved_images().size() > 0) {

        //this should be generated in the camera with those names
        cv::Mat plate = camera_device->get_next_plate();

        printf("size of queue: %d \n",camera_device->get_saved_images().size());
        camera_device->pop_camera();
        std::vector<uchar> imbuff;
        
        std::vector<int> compression_params;
        compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);

        imencode(".png",plate,imbuff,compression_params);

        std::vector<char> char_buff;
        char_buff.assign(imbuff.data(),imbuff.data() + imbuff.size());       

        printf("imbuff size: %d", imbuff.size());
        int buff_size = plate.rows * plate.cols * sizeof(uint8_t)*3;  
        std::vector<char> imbuff_vec(plate.rows * plate.cols * sizeof(uint8_t)*3);
        if (plate.isContinuous()) imbuff_vec.assign(plate.data,plate.data + plate.total()*3 );
        else exit(0);
         
        cv::Mat check(plate.rows,plate.cols,CV_8UC3,&imbuff_vec[0]);

        imshow("test",check);
 
        alpr::AlprResults results = openalpr.recognize(char_buff);

        printf("\n before loop  \n");
        if(results.plates.size() <= 0) continue;
       
         
        std::string image_name = upload_file(char_buff.data(),char_buff.size());
        using json = nlohmann::json;


        json resp_json = json::parse(image_name);

        std::string current_fname = resp_json["filename"];
        printf( "\n response:%s \n", current_fname.c_str());
        
        

        for (int i = 0; i < results.plates.size();i++){
            printf("\n ---------------------------------------- plate found --------------------------- \n");

            alpr::AlprPlateResult plate_alpr = results.plates[i];
            std::cout << "\n plate" << i << ":" << plate_alpr.topNPlates.size() << "result " << std::endl;

            detected_plates.push_back(plate_alpr.topNPlates[0]);
            printf("\n found plate: %s\n", plate_alpr.topNPlates[0].characters);

            //write_to_disk(plate,i);
                         
            }

    }

    if (detected_plates.size() > 0 ) {
        //alpr::AlprPlate best_fit = detected_plates[0];

        //for(int i = 0; i < detected_plates.size(); i++  ){
        //    if (best_fit.overall_confidence < detected_plates[i].overall_confidence){
        //            best_fit = detected_plates[i];

        //        }

       // }
      // for (int j =0; j < imp_veh.size(); j++) {

       //    if(imp_veh[j] == best_fit.characters) {
       //        printf("\n\n\n\n\n FLAGGED VEHICLE!!!!!!!!!!!!\n\n\n\n\n");
       //        flagged = 1;
      //     }

        //             }
        //std::cout << "best fit is :" << best_fit.characters << std::endl;
        if (test_conn()){

            for(int j = 0; j < detected_plates.size();j++){
        
                upload_data_live(detected_plates[j].characters, flagged, std::to_string(detected_plates[j].overall_confidence));
                //upload_file(char_buff, buff_size);
            }
        //store files
        printf("uploading files\n");
       // for (int l = 0; l < image_names.size(); l++){
       // }

        } else {

     //   upload_data_backup(best_fit.characters, flagged,std::to_string(best_fit.overall_confidence));
      //  for (int l = 0; l < image_names.size(); l++){
        //    store_file_to_backup(image_names[l]);
        //    }

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
