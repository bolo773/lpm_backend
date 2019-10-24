#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <sqlite3.h>


static int callback (void * data, int argc, char ** argv, char **azColName){
nlohmann::json tags_json;
int i;
fprintf(stderr, "%s",(const char *) data);
for(i=0;i<argc;i++){

//printf( "%s = %s",  azColName[i], argv[i] ? argv[i] : "NULL");
tags_json[azColName[i]] = argv[i] ? argv[i] : "NULL";

}
std::string fina = tags_json.dump();
std::cout << fina;
return 0;
}

using json = nlohmann::json;
int main(){
char * zErrMsg = 0;
json tag;
sqlite3 * db;
int rc;
const char * data = "we called the callback";
rc = sqlite3_open("backup.db",&db);
sqlite3_exec(db,"select * from raw_backup",callback,(void * ) data,&zErrMsg);

tag["plate_number"] = "ERK 433";
tag["date"] = "11/22/1111";
tag["image"] = "something.png";
std::string fina = tag.dump();
return 0;
}
