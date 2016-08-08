#include "devices_counters.h"

    /* --- impl of the class devices_counters --- */
    
    //init singleton
    devices_counters devices_counters::instance = devices_counters();

    //constructor
    devices_counters::devices_counters(){}

    //destructor
    devices_counters::~devices_counters(){}

    //get the singleton
    devices_counters& devices_counters::get_counters()
    {
        return instance;
    }

    //increment the counter
    void devices_counters::increment_counter(std::string id_device)
    {
        //lock
        protection_counters.lock();   
        
        if (counters.count(id_device) > 0 )
        {
            //data on the counters
            uint32_t tmp = counters[id_device];
            counters.erase(id_device);
            counters[id_device] = tmp +1;

        }
        else
        {
            //no data on the counters
            counters[id_device] = 1;
        }

        //unlock
        protection_counters.unlock();
    }

    std::string devices_counters::get_string_counters_values()
    {
        std::string ret;

        if (counters.size() == 0)
        {
            ret = "Nothing to show";
        }
        else
        {
            std::map<std::string,uint32_t>::iterator it;
            for(it=counters.begin(); it != counters.end(); ++it)
            {
                std::string name = it->first;
                std::string value = std::to_string(it->second);

                ret +=" " + name + " : " + value + " |";
            }
        }

        return ret;
    }

    std::string devices_counters::get_JSON_counters_values()
    {
        std::string ret;

        if (counters.size() == 0)
        {
            ret = "[]";
        }
        else
        {
            ret = "[";

            std::map<std::string,uint32_t>::iterator it;
            for(it=counters.begin(); it != counters.end(); ++it)
            {
                std::string id = it->first;
                std::string value = std::to_string(it->second);

                ret +="{\"id\":\"" + id + "\",\"val\":\"" + value + "\"},";

            }
            //delete the last char
            ret = ret.substr(0, ret.size()-1);
            ret +="]";
        }

        return ret;
    }
