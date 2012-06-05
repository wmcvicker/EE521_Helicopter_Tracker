#include <SoftwareSerial.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>							// Standard C library
#include <avr/io.h>	
#include <string.h>						// Input-output ports, special registers

#define rxPin 2
#define txPin 3
#define ledPin 13

typedef struct packet {
   uint8_t dlen;
   uint8_t dev_id;
   uint8_t ptype;
   uint8_t data[24];
} packet_t;


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

 #define BAUD_PRESCALE 103

// set up a new serial port
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

typedef struct {
  uint8_t tag[6];
  uint8_t junk1[16];
  uint8_t lattitude[10];
  uint8_t junk2[2];
  uint8_t longitude[10];
  uint8_t junk3[3];
  uint8_t velocity[5];
  uint8_t junk4;
  uint8_t course[5];
} GPRMC;

void usart_init(uint16_t baudin, uint32_t clk_speedin)
{
	uint32_t ubrr = (clk_speedin/16UL)/baudin-1;
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;

	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);

	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
	UCSR0A &= ~(1<<U2X0);
}

/*the send function will put 8bits on the trans line. 
If the receiver is not listening it will broadcast to 
nothing and you wont know.
	@data	the 8 bits you are sending
*/
void usart_send( uint8_t data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

/* the receive data function. Note that this a blocking call
Therefore you may not get control back after this is called 
until a much later time. It may be helpfull to use the 
istheredata() function to check before calling this function
	@return 8bit data packet from sender
*/
uint8_t  usart_recv(void)
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}

/* function check to see if there is data to be received
	@return true is there is data ready to be read
*/
uint8_t  usart_istheredata(void)
{
	 
	return (UCSR0A & (1<<RXC0));
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

	for(i = 0; i < 3 + len; i++)
		usart_send(packet_ptr[i]);
	
}


void setup()  {
  // GPS
  // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  Serial.begin(9600);
  // uart to comp

  usart_init(9600, 16000000 ); 
  
  Serial.write( 0xAA );
  Serial.write( 0x55 );
  
  EIMSK |= ( 1 << INT0);
  // Signal change triggers interrupt
  MCUCR |= ( 1 << ISC00);
  MCUCR |= ( 0 << ISC01);
  sei();
  
  
}

char data;
int i = 0;
int j = 0;
char search[6] = {'$', 'G', 'P', 'R', 'M', 'C'};
char string[57];
int valid = 0;
GPRMC *gps;
char *gps_data;

void loop() {
  // listen for new serial coming in:
//  char search [5] = {'G', 'P', 'G', 'G', 'A'};
//  int sizeofsearch = 5;            //size of array
//  char GPSstring [70];
//  int i = 0;
//  int j = 0;
//  
//  while(1)
//  {
//    if( search[i] == (data = mySerial.read() ) )
//    {
//        i++;
//        if( i == sizeofsearch)            //search string found
//        {
//           while((data = mySerial.read()) != '\n')          //saves string to array
//           {
//             if(data == -1)
//             {
//               continue;
//             }
//             GPSstring[i] = data;
//             i++;    
//           }
//           Serial.println(GPSstring); 
//        }
//    }
//    else if ( data == -1)  // nothing on port yet
//    {
//       continue; 
//    }
//    
//    else                  // garbage restart search
//    {
//      i = 0;
//    }
//  } 
    
  
}


ISR(INT0_vect){
   data = mySerial.read();
   if(data == search[i]){
      i++; 
   }
   string[j] = data;
   
   if(i == 6 && j == 59){
      valid = 1;
      gps = (GPRMC *)string;
      gps->lattitude[9] = '\0';
      gps->longitude[9] = '\0';
      gps_data = (char *)gps->lattitude;
      gps_data += 9;
      gps_data = (char *)gps->longitude;
      gps_data -= 9;
      usart_send_to_MSP(GPS_DATA_T, (uint8_t *)gps_data, 18);
    
   }
   
   if(data == '\r'){
       j = 0;
       i = 0; 
   }
  
   j++;  
   
}
  



 


