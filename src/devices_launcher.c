#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "device_types.h"

#define NB_DEVICES_MAX_C 5
#define LAUNCH_ERROR_C 127

/*-------------------------------------------------*/
/*  This program launch few devices for simulation */
/*-------------------------------------------------*/

            /* ---- global variables ---- */

    //id of the device
    pid_t devices_pid[NB_DEVICES_MAX_C];
    uint8_t nb_devices = 0;              

            /* ---- prototypes of internal functions ---- */

    // Catch the SIGINT signal to kill all the devices
    void signal_handler(int sig);


            /* ---- impl of main ---- */ 

    int main(void)
    {
        char id_device[ID_LENGTH_C];
        
        //Change the action for a kill signal to have a "clean" end
        signal(SIGINT, signal_handler);

        for(uint8_t device=0;device < NB_DEVICES_MAX_C; device++)
        {
            pid_t pid = fork();
            
/* ----  device code ---- */

            if(pid == 0)
            {
                //generated the device name
                sprintf(id_device, "DEV%d", device+1);

                //launch the device
                execl("device", "device", id_device, NULL);
                
               /* !!! end of children !!! */
               printf("Launch error: %s\n",id_device);
               exit(LAUNCH_ERROR_C);
            }

/* ----  launcher code ---- */

            //store the pid
            devices_pid[device] = pid;
            nb_devices++;
        }

        //wait all the devices
        while (wait(NULL) > 0);
        printf("Launcher close.\n");
        exit(0);
    }

            /* ---- impl of internal functions ---- */

    //Stop the program
    void signal_handler(int sig)
    {
        if(sig == SIGINT){
            //kill all the devices
            for(uint8_t device=0;device < nb_devices; device++)
            {
                kill(devices_pid[device], SIGINT);
            }
            sleep(1);
            exit(0);
        }
    }
