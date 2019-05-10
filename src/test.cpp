#include "TTcore.hpp" 
int main (int argc, char *argv[]){
/*
camera* cam1 = new camera(1,0,0);

cam1->monitor();
cam1->grab_images();
*/

std::istringstream ss(argv[1]);
int x;
if(!(ss >> x)){
    std::cout << " must define camera index \n";
    
}

TTcore manager(x);
manager.start();

//monitor_instance firstmonitor(1,1);

//firstmonitor.monitor();




}
