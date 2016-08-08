#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/SocketAddress.h>

#include "devices_counters.h"
#include "device_types.h"

/*----------------------------------------------------*/
/*  This program count the data and show the counter  */
/*----------------------------------------------------*/

           /* ---- prototypes of internal functions ---- */

    // Print the counters values on stdout each second
    void print_counters();

    //accept the connections
    void tcp_server();

    //receive the measurements
    void tcp_receive(Poco::Net::StreamSocket socket);

    //accept the connections for web server
    void tcp_webserver();

    //sent data to web client in json format
    void tcp_send_data(Poco::Net::StreamSocket socket);

            /* ---- impl of main ---- */ 

int main()
 {
    std::thread thread_print_counters(print_counters);
    std::thread thread_tcp_server(tcp_server);
    std::thread thread_tcp_webserver(tcp_webserver);


    thread_print_counters.join();

}

           /* ---- impl of internal functions ---- */ 

    // Print the counters values on stdout
    void print_counters()
    {
        while(true)
        {
            std::this_thread::sleep_for (std::chrono::seconds(1));
            std::cout << devices_counters::get_counters().get_string_counters_values() << std::endl;
        }
    }

    //accept the connections
    void tcp_server()
    {
        Poco::Net::ServerSocket srv(5001);

        while(true)
        {
            Poco::Net::StreamSocket socket = srv.acceptConnection();
            std::thread (tcp_receive,socket).detach();           
        }
    }

    //receive the measurements
    void tcp_receive(Poco::Net::StreamSocket socket)
    {
            
            //get all the data
            bool exit = false;

            data_flow_unit_t data;

            //block empty
            char empty_id[ID_LENGTH_C +1];
            for(int i=0; i<ID_LENGTH_C +1; i++)
                empty_id[i]=0;

            while(!exit)
            {
                socket.receiveBytes((void *)&data,sizeof(data_flow_unit_t),0);

                //empty => exit
                if(memcmp(data.device_id,empty_id,ID_LENGTH_C +1) == 0)
                {
                    exit = true;
                }
                else
                {
                    //change the id to string
                    std::string device_id_str(data.device_id);

                    //increase the counter
                    devices_counters::get_counters().increment_counter(device_id_str);
                    
                }
            }
    }

    //accept the connections for web server
    void tcp_webserver()
    {
        Poco::Net::ServerSocket srv(8081);

        while(true)
        {
            Poco::Net::StreamSocket socket = srv.acceptConnection();
            std::thread (tcp_send_data,socket).detach();           
        }
    }

    //sent data to web client in json format
    void tcp_send_data(Poco::Net::StreamSocket socket)
    {
        Poco::Net::SocketStream str(socket);
        str << "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: application/json\r\n"
            "\r\n"
            +devices_counters::get_counters().get_JSON_counters_values()
            << std::flush;
    }

     
