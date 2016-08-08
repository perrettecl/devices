#ifndef DEVICES_COUNTERS_H
#define DEVICES_COUNTERS_H

#include <map>
#include <string>
#include <cstdint>
#include <mutex>

//devices_counters : singleton

class devices_counters
{
  private:

    /* --- Attributs --- */
    static devices_counters instance; //singleton 
    
    std::map<std::string,uint32_t> counters; //list of counters
    
    std::mutex protection_counters; //protection
       
    //make the constructor and the copy private
    devices_counters();
    ~devices_counters();

    devices_counters(const devices_counters&){}
    devices_counters& operator = (const devices_counters&){}

  public:
    static devices_counters& get_counters(); //get the singleton

    void increment_counter(std::string id_device); //increment one counter

    std::string get_string_counters_values(); // get a string with the value of each counter
    std::string get_JSON_counters_values(); // get a string in JSON format with the value of each counter
};
#endif
