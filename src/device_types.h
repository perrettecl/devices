#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define ID_LENGTH_C 7

    //type for one measurement
    typedef struct {
        uint32_t data;
    } measurement_t;

    //type for data flow unit
    // data flow format: |id_device|measurement|
    typedef struct {
        char device_id[ID_LENGTH_C +1];
        measurement_t measurement;
    } data_flow_unit_t;

    //type byte
    typedef unsigned char byte_t;


#endif
