
#ifndef COMMON_H
#define COMMON_H

#define HEADER_SIZE 3 
#define HELICOPTER_ID 18
#define BS_ID 1

#define INIT_T 0
#define GPS_DATA_T 1
#define SERVO_DATA_T 2

/////////////////// States ////////////////////
#define IDLE_S 1
#define GET_UART_S 2
#define GET_RF_S 3


struct packet {
   uint8_t dlen;
   uint8_t dev_id;
   uint8_t ptype;
   uint8_t data[50];
} __attribute__((__packed__));
typedef struct packet packet_t;

struct GPS_Data {
   uint16_t lat_deg;
   uint8_t lat_min;
   uint16_t lat_milli_min;
   uint16_t lon_deg;
   uint8_t lon_min;
   uint16_t lon_milli_min;
   uint8_t quad; // NE, NW, SE, SW
} __attribute__((__packed__));
typedef struct GPS_data gps_t;

#endif
