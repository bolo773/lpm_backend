#include "veh_entry.hpp"


std::string veh_entry::get_number(){

    return this->number;


}

std::string veh_entry::get_accuracy(){

    return this->accuracy;

}


bool veh_entry::is_flagged(){

    return this->flagged;

}

veh_entry::veh_entry( std::string number, std::string accuracy, std::string flagged){
    this->number = number;
    this->accuracy = accuracy;
    this->flagged = flagged;

}
