#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <Wire.h>
#include <Servo.h>
#include "i2c.c"

#define BMP085_ADDRESS 0x77  // I2C address of BMP085

///////////////////////// Constants for MSP430 comm ////////////////////////////

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


// structure for MSP430 comms packet
typedef struct packet {
   uint8_t dlen;
   uint8_t dev_id;
   uint8_t ptype;
   uint8_t data[24];
} packet_t;


const unsigned char OSS = 0;  // Oversampling Setting

// Calibration values for pressure sensor
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;

// b5 is calculated in bmp085GetTemperature(), this variable is also used in bmp085GetPressure()
// so Temperature() must be called before Pressure()
long b5; 

short temperature;
long pressure;
const float p0 = 101325;     // Pressure at sea level (Pa)
float altitude;

// accel component variables
double xAccelComp, zAccelComp, yAccelComp;
double prevX, prevY, prevZ;

// compass variables
int HMC6352Address = 0x42;
// This is calculated in the setup() function
int slaveAddress;
uint16_t heading;
uint16_t recHeading;
uint8_t recHeadingLow;
uint8_t recHeadingHigh;

volatile int count = 0;
volatile int pulseWidth = 0;

/////////////////////////// Functions for accel/gyro ///////////////////////////

// Custom i2c function for the accel/gyro
// Reads the register corresponding to reg_add from device dev_add
// Returns regiter's 8 bit value 
unsigned char read_reg(unsigned char dev_add, unsigned char reg_add)
{
    unsigned char data;
    
    if(i2c_start(dev_add << 1 | I2C_WRITE))
       _delay_ms(1);

    if(i2c_write(reg_add))
       _delay_ms(1);
       
    if(i2c_rep_start(dev_add << 1 | I2C_READ))
       _delay_ms(1);
       
    data = i2c_readNak();
    
    i2c_stop();
    
    _delay_ms(1);
    
    return data;
}

// Custom i2c function for the accel/gyro
// Writes data to the register corresponding to reg_add from device dev_add
// Returns void 
void write_reg(unsigned char dev_add, unsigned char reg_add, unsigned char data)
{
  if(i2c_start(dev_add << 1 | I2C_WRITE))
       _delay_us(1);

  if(i2c_write(reg_add))
       _delay_us(1);
       
  if(i2c_write(data))
       _delay_us(1);
  
  i2c_stop();
  
  _delay_ms(1);
}

double getXaccelComp(double prevXcomp){
   char xLow, xHigh; 
   double xMag;
   double retValue;
   int count = 0;
   
   do{
      xLow = read_reg(0x68, 60);
      xHigh = read_reg(0x68, 59);
      retValue = (double)(xHigh << 8 | xLow) / 32768 * 19.62;
      xMag = (double)abs(retValue); // convert to g
      _delay_us(100);
      count++;
      if(count > 100){
        break; 
      }
   }while(xMag < .2 * abs(prevXcomp));

   return retValue;
}

double getYaccelComp(double prevYcomp){
   char yLow, yHigh; 
   double yMag;
   double retValue;
   int count = 0;
   
   do{
      yLow= read_reg(0x68, 62);
      yHigh = read_reg(0x68, 61);
      retValue = (double)(yHigh << 8 | yLow) / 32768 * 19.62;
      yMag = (double)abs(retValue); // convert to g
      _delay_us(100);
      count++;
      if(count > 100){
        break; 
      }
   }while(yMag < .2 * abs(prevYcomp));

   return retValue;
}

double getZaccelComp(double prevZcomp){
   char zLow, zHigh; 
   double zMag;
   double retValue;
   int count = 0;
   
   do{
      zLow = read_reg(0x68, 64);
      zHigh = read_reg(0x68, 63);
      retValue = (double)(zHigh << 8 | zLow) / 32768 * 19.62;
      zMag = (double)abs(retValue); // convert to g
      _delay_us(100);
      count++;
      if(count > 100){
        break; 
      }
   }while(zMag < .2 * abs(prevZcomp));

   return retValue;
}

void initAccelCompValues(double *x, double *y, double *z){
   char xLow, xHigh, yLow, yHigh, zLow, zHigh;
   
   xLow = read_reg(0x68, 60);
   xHigh = read_reg(0x68, 59);
   yLow= read_reg(0x68, 62);
   yHigh = read_reg(0x68, 61); 
   zLow = read_reg(0x68, 64);
   zHigh = read_reg(0x68, 63);
      
   *x = (double)(xHigh << 8 | xLow) / 32768 * 19.62;
   *y = (double)(yHigh << 8 | yLow) / 32768 * 19.62;
   *z = (double)(zHigh << 8 | zLow) / 32768 * 19.62;
}

/////////////////////// Functions for pressure sensor //////////////////////////

// Modified from Jim Lindblom's example code 
// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Modified from Jim Lindblom's example code 
// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut)
{
  long x1, x2;
  
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Modified from Jim Lindblom's example code 
// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}

// Modified from Jim Lindblom's example code 
// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(address);
  Wire.endTransmission();
  
  PORTB = 0xFF;
  Wire.requestFrom(BMP085_ADDRESS, 1);
  PORTB = 00;
  while(!Wire.available())
    ;
    
  return Wire.receive();
}

// Modified from Jim Lindblom's example code 
// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.receive();
  lsb = Wire.receive();
  
  return (int) msb<<8 | lsb;
}

// Modified from Jim Lindblom's example code 
// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF4);
  Wire.send(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(5);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Modified from Jim Lindblom's example code 
// Read the uncompensated pressure value
unsigned long bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF4);
  Wire.send(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3);
  msb = Wire.receive();
  lsb = Wire.receive();
  xlsb = Wire.receive();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}

/////////////////////////// Functions for compass ////////////////////////////

int getHeading(){
   byte headingData[2];
   int i, headingValue;
   double retValue;
   
   Wire.beginTransmission(slaveAddress);
   Wire.send("A");              // The "Get Data" command
   Wire.endTransmission();
   delay(10);                   // The HMC6352 needs at least a 70us (microsecond) delay
   Wire.requestFrom(slaveAddress, 2);        // Request the 2 byte heading (MSB comes first)
   i = 0;
   while(Wire.available() && i < 2)
   { 
      headingData[i] = Wire.receive();
      i++;
   }
   
   headingValue = headingData[0]*256 + headingData[1];  // Put the MSB and LSB together
  
   retValue = (double)headingValue / 10;
   
   return headingValue;
}
//////////////////////////// Functions for gps //////////////////////////////



/////////////////////////// MSP430 com functions ////////////////////////////

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
   
   usart_send(0xAA);
   usart_send(0x55);
   for(i = 0; i < 3 + len; i++)
      usart_send(packet_ptr[i]);
	
}

Servo servo1;
Servo servo2;

void setup()
{
  DDRC = 0xFF;
  DDRB = 0xFF;

  //sei();
  
  slaveAddress = HMC6352Address >> 1;
  
  servo1.attach(9);
  servo1.writeMicroseconds(1500);
  
  servo2.attach(10);
  servo2.writeMicroseconds(1500);
  
  Wire.begin();  // init i2c for pressure and compass sensor
  bmp085Calibration();  // calibrate pressure sensor
  
  i2c_init();  // init i2c for accel/gyro sensor
  Serial.begin(9600);  // init usart

  usart_init(9600, 16000000);
  
  _delay_ms(3000);
  
  // initialization for accel/gyro sensor
  write_reg(0x68, 107, 0x20);
  write_reg(0x68, 108, 0xC0);
  write_reg(0x68, 28, 0x00);
  // end initialization
  
  initAccelCompValues(&xAccelComp, &yAccelComp, &zAccelComp);
  prevX = xAccelComp;
  prevY = yAccelComp;
  prevZ = zAccelComp;
}




uint8_t data[5];
int i;
int headingDiff;

void loop(){

    for(i=0;i<5;i++)
    {
       data[i] = 0x00; 
    }
  
    // read from pressure sensor and display
    temperature = bmp085GetTemperature(bmp085ReadUT());
    pressure = bmp085GetPressure(bmp085ReadUP());
    
    // convert pressure reading to altitude and display
    altitude = (float)44330 * (1 - pow(((float) pressure/p0), 0.190295));
    
    // read accel data and display
    xAccelComp = getXaccelComp(xAccelComp);
    yAccelComp = getYaccelComp(yAccelComp);
    zAccelComp = getZaccelComp(zAccelComp);
    
    if(xAccelComp > .5){
      //Serial.println("Drifting in +x");
      //data[0] = 'X';
      servo1.writeMicroseconds(1100);
    }
    else if(xAccelComp < -.5){
      //Serial.println("Drifting in -x");
      //data[0] = 'x';
      servo1.writeMicroseconds(1900);
    }
    else
    {
      servo1.writeMicroseconds(1500);
    }
    
    if(yAccelComp > .5){
      //Serial.println("Drifting in +y");
      //data[1] = 'Y';
      servo2.writeMicroseconds(1100);
    }
    else if(yAccelComp < -.5){
      //Serial.println("Drifting in -y");
      //data[1] = 'y';
      servo2.writeMicroseconds(1900);
    }
    else
    {
      servo2.writeMicroseconds(1500); 
    }
    
    if(zAccelComp - prevZ > .5){
      //Serial.println("Drifting in +z");
      //data[2] = 'Z';
    }
    else if(zAccelComp - prevZ < -.5){
      //Serial.println("Drifting in -z");
      //data[2] = 'z';
    }
  
    // read compass sensor and display value
    heading = getHeading();

    data[0] = (heading >> 8) & 0xff;
    data[1] = heading & 0xff;    
    
    usart_send_to_MSP(SERVO_CTRL_T, data, 2);
    
   _delay_ms(100); // do this about once a second
    
}




