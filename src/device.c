#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "device_types.h"

#define ERROR_PARAM_C 1
#define ERROR_BUFFER_FULL_C 2
#define ERROR_NETWORK_C 3

#define BUFFER_SIZE_C 5

#define LISTEN_PORT 5001

/*-------------------------------------------------*/
/*  This program launch one device for simulation  */
/*-------------------------------------------------*/

            /* ---- global variables ---- */

    //id of the device
    char device_id[ID_LENGTH_C +1];              
    
    // Buffer of measurements
    measurement_t* buffer;
    uint32_t nb_measurements = 0;

    // statistics about measurements
    uint32_t nb_measurements_genereted = 0;
    uint32_t nb_measurements_sent = 0;

    //threads
    pthread_t thread_simulation_measurements;
    pthread_t thread_send_measurements;

    //shared protection and sync
    pthread_cond_t synchro = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
    


            /* ---- prototypes of internal functions ---- */

    // Indicates to each thread to terminate after:
    // -> genereted the last value for thread_simulation_measurements
    // -> sent all the measurements in the buffer for thread_send_measurements
    void signal_handler(int sig);

    // || Thread || Generates the value for measurement and store this measurement in the buffer
    // error case: Buffer is full -> stop the program
    void * simulation_measurements();

    // || Thread || Send the measurements from the buffer
    // call: send_data_flow
    // error case: Impossible to send the data after 3 retry -> stop the program 
    void * send_measurements();

    // Create a connection and send the data flow of measurements from the buffer to the server
    // return : true if no errorm
    bool send_data_flow(data_flow_unit_t* data_flow, uint32_t nb_data);

            /* ---- impl of main ---- */ 

    int main(int argc, char **argv)
    {
        //check params
        if(argc != 2 || strlen(argv[1]) > ID_LENGTH_C){
            fprintf(stderr, "Param error\n");
            exit(ERROR_PARAM_C);
        }

        //copy the device_id
        strcpy(device_id,argv[1]);
        
        //Change the action for a kill signal to have a "clean" end
        signal(SIGINT, signal_handler);

        //init the buffer
        buffer = (measurement_t*) malloc(BUFFER_SIZE_C * sizeof(measurement_t));

        //Create the threads
        pthread_create(& thread_simulation_measurements, NULL,simulation_measurements, NULL);
        pthread_create(& thread_send_measurements, NULL,send_measurements, NULL);

        printf("%s running.\n", device_id);

        //Wait until the end of the thread execution
        pthread_join (thread_simulation_measurements, NULL);
        pthread_join (thread_send_measurements, NULL);

    }

            /* ---- impl of internal functions ---- */ 

    //Stop the program
    void signal_handler(int sig)
    {
        if(sig == SIGINT){
            //Print the statistics
            printf("\nEnd of %s: %d sent for %d genereted\n", device_id,nb_measurements_sent,nb_measurements_genereted);

            //terminate
            free(buffer);
            exit(0);
        }
    }

    // Generates the value for measurement and store this measurement in the buffer
    // error case: Buffer is full -> stop the program
    void * simulation_measurements()
    {
        //init random generator
        srand(getpid());

        measurement_t measurement_current;

        while(true)
        {
            //generate the measurement
            measurement_current.data = (uint32_t) rand();

            pthread_mutex_lock(& mutex_buffer);
            
                //error: The buffer is full -> We stop the program
                if(nb_measurements == BUFFER_SIZE_C){

                    //kill thread_send_measurements
                    pthread_cancel(thread_send_measurements);
                
                    //exit
                    printf("!!! BUFFER FULL %s: %d sent for %d genereted\n", device_id,nb_measurements_sent,nb_measurements_genereted);
                    exit(ERROR_BUFFER_FULL_C);
                }

                //store the measurement
                buffer[nb_measurements] = measurement_current;
                nb_measurements++;            

            pthread_mutex_unlock(& mutex_buffer);

            nb_measurements_genereted++;

            // wake up the thread in  charge to send the data
            pthread_cond_signal (& synchro);

            //sleep until the next generation - between 1ms to 2000ms, 10ms step
            uint32_t sleep_time = (uint32_t) rand();
            sleep_time = sleep_time % 2000;
            usleep(sleep_time * 1000);
        }
    }

    // Send the measurements from the buffer
    // error case: Impossible to send  after 3 retry -> stop the program
    void * send_measurements()
    {   
        //local buffer
        measurement_t local_buffer[BUFFER_SIZE_C];
        uint32_t nb_measurement_local =0;

        while(true)
        {

            
            pthread_mutex_lock(& mutex_buffer);

                //until the buffer is empty, wait
                while(nb_measurements <= 0)
                {
                    //wait until the generator wake me up
                    pthread_cond_wait (& synchro, & mutex_buffer);
                }

            
                //cpoy all the measurements from the buffer
                memcpy(local_buffer, buffer, sizeof(measurement_t)*BUFFER_SIZE_C);

                //for(int i=0; i < BUFFER_SIZE_C; i++) {printf("%d ",buffer[i].data);} printf("\n %d \n",nb_measurements);//#DEBUG print buffer
                //for(int i=0; i < BUFFER_SIZE_C; i++) {printf("%d ",local_buffer[i].data);} printf("\n");//#DEBUG print buffer

                nb_measurement_local = nb_measurements;   
                nb_measurements = 0;   

            pthread_mutex_unlock(& mutex_buffer);
                
            // create the data flow to send the measurements to the server
            data_flow_unit_t* data_flow = (data_flow_unit_t*)malloc(nb_measurement_local * sizeof(data_flow_unit_t));

            //copy the measurements on the data flow
            for(uint32_t measurement=0; measurement < nb_measurement_local; measurement++){
                data_flow[measurement].measurement = local_buffer[measurement];
                strcpy(data_flow[measurement].device_id,device_id);
            }

            //for(int i=0; i < nb_measurement_local; i++) {printf("%s | %d\n",data_flow[i].device_id, data_flow[i].measurement.data );} printf("\n");//#DEBUG show the data flow

            //send the mesurements to the server
            bool job_done = false;
            uint8_t nb_try=0;

            while(nb_try < 3 && !job_done)
            {
                nb_try++;
                job_done = send_data_flow(data_flow, nb_measurement_local);
            }

            free(data_flow);

            //error: impossinble to send the data
            if(!job_done)
            {
                    //kill thread_send_measurements
                    pthread_cancel(thread_simulation_measurements);
                
                    //exit
                    printf("!!! NETWORK ERROR %s: %d sent for %d genereted\n", device_id,nb_measurements_sent,nb_measurements_genereted);
                    exit(ERROR_NETWORK_C);
            }

            nb_measurements_sent += nb_measurement_local;

            //sleep(6); //#DEBUG saturation du buffer
  
        }
    }

    // Create a connection and send the data flow of measurements from the buffer to the server
    // return : true if no error
    bool send_data_flow(data_flow_unit_t* data_flow, uint32_t nb_data)
    {
        //init the connection
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in socket_in = { 0 };

        
        inet_aton("localhost", &socket_in.sin_addr);
        socket_in.sin_port = htons(LISTEN_PORT);
        socket_in.sin_family = AF_INET;
        
        //connection
        int error;
        error = connect(sock,(struct sockaddr *) &socket_in, sizeof(struct sockaddr));
        if(error < 0)return false;

        // create the buffer to send the data and add the close signal for the server (data_flow_unit = {0})
        uint32_t data_size = (nb_data + 1)* sizeof(data_flow_unit_t);
        byte_t* data = (byte_t*) calloc(nb_data + 1,sizeof(data_flow_unit_t));

        memcpy(data,data_flow,nb_data * sizeof(data_flow_unit_t));

        //send the buffer
        int data_sent;
        data_sent = send(sock, data, data_size, 0);

        close(sock);
        free(data);

        //error
        if(data_sent < 0)return false;

        return true;
    }
    




