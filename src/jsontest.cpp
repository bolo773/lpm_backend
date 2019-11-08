#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <sqlite3.h>


static int callback (void * res,int argc, char ** argv, char **azColName){

static int j = 0;

static nlohmann::json tags_json;
int i;
for(i=0;i<argc;i++){
    

//printf( "%s = %s",  azColName[i], argv[i] ? argv[i] : "NULL");
tags_json[j][azColName[i]] = argv[i] ? argv[i] : "NULL";
}
std::string fina = tags_json.dump();
*(std::string *)res = fina;

j++;
return 0;

}

using json = nlohmann::json;
int main(){
char * zErrMsg = 0;
json tag;
sqlite3 * db;
int rc;
rc = sqlite3_open("backup.db",&db);
int j = 0;
std::string fina;
sqlite3_exec(db,"select * from raw_backup inner join images_backup on ",callback,(void*)&fina,&zErrMsg);

std::cout << fina;
return 0;
}
