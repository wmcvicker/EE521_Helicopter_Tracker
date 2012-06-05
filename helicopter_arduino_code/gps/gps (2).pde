#include <SoftwareSerial.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define rxPin 2
#define txPin 3
#define ledPin 13

#define INIT_T 0
	//Used to initialize network communication

#define GPS_DATA_T 1
	//Packet contains GPS data

#define SERVO_CTRL_T 2
	//Packet contains servo controls

#define REQ_STATUS_T 3
	//Request for device’s status

#define STATUS_T 4
	//Packet contains device’s status

#define NEW_MISSION_T 5
	//Send destination GPS coordinates for the mission

#define MISSION_COMP_T 6
	//Destination has been reached

#define MISSION_ABORT_T 7
	//From Helicopter    ->  mission has been aborted
	//From Base Station  ->  tell helicopter to abort mission

#define ACCEL_DATA_T 8
	//Packet contains Accel data

#define GYRO_DATA_T 9
	//Packet contains Gyro data

#define BAROM_DATA_T 10
	//Packet contains Barometer data

#define ALTIM_DATA_T 11
	//Packet contains Altimeter data

// structure for MSP430 comms packet
typedef struct packet {
   uint8_t dlen;
   uint8_t dev_id;
   uint8_t ptype;
   uint8_t data[24];
} packet_t;

// set up a new serial port
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

typedef struct {
  uint8_t tag[6];
  uint8_t junk1[16];
  uint8_t lattitude[10];
  uint8_t junk2[3];
  uint8_t longitude[10];
  uint8_t junk3[3];
  uint8_t velocity[5];
  uint8_t junk4;
  uint8_t course[5];
} GPRMC;


void setup()  {
  // GPS
  // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  
  // uart to comp
  Serial.begin(9600);
  // prints title with ending line break
  Serial.println("GPS data"); 
  
}

char data;
int i = 0;
int j = 0;
char search[6] = {'$', 'G', 'P', 'R', 'M', 'C'};
char string[81];
volatile uint8_t valid;
GPRMC *gps;

char sendstring[31];

void strcopy( char* source, char* destination, uint8_t num)
{
   uint8_t i = 0;
  
   while(i != num)
   {
      destination[i] = source[i];
      i++;
   }
   
   return;
  
}


void get_GPS()
{
  // turn on GPS interrupts
  EIMSK |= ( 1 << INT0);
  // Signal change triggers interrupt
  MCUCR |= ( 1 << ISC00);
  MCUCR |= ( 0 << ISC01); 
  

  
  valid = 0;              //variable used to wait     
  
  sei();
  //Serial.println("looping");
  while(valid == 0)
  {
  }
  cli();
  //Serial.println("got here");
  
  strcopy((char*)gps->lattitude, sendstring, 10);
  //sendstring[10] = '$';
  strcopy((char*)gps->longitude, sendstring + 10, 10);
  //sendstring[21] = '$';
  strcopy((char*)gps->velocity, sendstring + 20, 5);
  //sendstring[27] = '$';
  strcopy((char*)gps->course, sendstring + 25, 4);
  
  //Serial.println(sendstring);
  
  return;
}


void usart_send_to_MSP(uint8_t packet_type, uint8_t sdata[], uint8_t len)
{
   packet_t thePacket;
   uint8_t *packet_ptr = (uint8_t *) &thePacket;
   uint8_t i;

   thePacket.dlen = len;
   thePacket.dev_id = 18;
   thePacket.ptype = packet_type;
   for(i=0;i<len;i++)
   {
      thePacket.data[i] = sdata[i];
   }

   /* Instead of just creating the packet then looping through it. Can't we
      just skip the step of creating? */
   
   Serial.write(0xAA);
   Serial.write(0x55);
   for(i = 0; i < 3 + len; i++)
      Serial.write(packet_ptr[i]);
	
}





void loop() { 
  
  get_GPS();
  usart_send_to_MSP(GPS_DATA_T, (uint8_t*) &sendstring , 31);
  
}




ISR(INT0_vect){
 

  data = mySerial.read();
   
   
   if(data == search[i]){
      i++; 
   }
   string[j] = data;
   
   if(i == 6 && j == 72){
      valid = 1;
      //  Serial.println(string);
      gps = (GPRMC *)string;        //catch string
//       // turn on GPS interrupts
//      EIMSK &= ~( 1 << INT0);
//      // Signal change triggers interrupt
//      MCUCR &= ~( 1 << ISC00);
//      MCUCR &= ~( 0 << ISC01); 
//      cli();
   }
   
   if(data == '\r'){
       j = 0;
       i = 0; 
   }
  
   j++;  
   
}
  


