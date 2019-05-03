#include "camera.hpp"
#include "alpr.h"

class monitor_instance {
    
    std::vector<std::string> imp_veh;

    camera * camera_device;
    char cwd[PATH_MAX];
    int livedb;
    
    public:

    int monitor();
    int analyze_plates();
    monitor_instance(int camera_index, int livedb);
    
};
