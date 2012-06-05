//#define F_CPU 16000000UL
#include <avr/io.h>
//#include <util/delay.h>
//#include <avr/interrupt.h>
//#include <util/twi.h>
//#include <stdlib.h>
#include <math.h>


#define I2C_READ    1
/** defines the data direction (writing to I2C device) in i2c_start(),i2c_rep_start() */
#define I2C_WRITE   0
/* I2C clock in Hz */
#define SCL_CLOCK  100000L
//Defines from Marek Michalkiewicz's awesome TWI.h
/* TWSR values (not bits) */
/* Master */
#define TW_START		0x08
#define TW_REP_START		0x10
/* Master Transmitter */
#define TW_MT_SLA_ACK		0x18
#define TW_MT_SLA_NACK		0x20
#define TW_MT_DATA_ACK		0x28
#define TW_MT_DATA_NACK		0x30
#define TW_MT_ARB_LOST		0x38
/* Master Receiver */
#define TW_MR_ARB_LOST		0x38
#define TW_MR_SLA_ACK		0x40
#define TW_MR_SLA_NACK		0x48
#define TW_MR_DATA_ACK		0x50
#define TW_MR_DATA_NACK		0x58
/* Slave Transmitter */
#define TW_ST_SLA_ACK		0xA8
#define TW_ST_ARB_LOST_SLA_ACK	0xB0
#define TW_ST_DATA_ACK		0xB8
#define TW_ST_DATA_NACK		0xC0
#define TW_ST_LAST_DATA		0xC8
/* Slave Receiver */
#define TW_SR_SLA_ACK		0x60
#define TW_SR_ARB_LOST_SLA_ACK	0x68
#define TW_SR_GCALL_ACK		0x70
#define TW_SR_ARB_LOST_GCALL_ACK 0x78
#define TW_SR_DATA_ACK		0x80
#define TW_SR_DATA_NACK		0x88
#define TW_SR_GCALL_DATA_ACK	0x90
#define TW_SR_GCALL_DATA_NACK	0x98
#define TW_SR_STOP		0xA0
/* Misc */
#define TW_NO_INFO		0xF8
#define TW_BUS_ERROR		0x00
/* The lower 3 bits of TWSR are reserved on the ATmega163.
 * The 2 LSB carry the prescaler bits on the newer ATmegas. */
#define TW_STATUS_MASK		(_BV(TWS7)|_BV(TWS6)|_BV(TWS5)|_BV(TWS4)|\
				_BV(TWS3))
#define TW_STATUS		(TWSR & TW_STATUS_MASK)
/*  R/~W bit in SLA+R/W address field.*/
#define TW_READ		1
#define TW_WRITE	0

/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void);

/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
unsigned char i2c_start(unsigned char address);

/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 Input:   address and transfer direction of I2C device
*************************************************************************/
void i2c_start_wait(unsigned char address);

/*************************************************************************
 Issues a repeated start condition and sends address and transfer direction 
 Input:   address and transfer direction of I2C device
 Return:  0 device accessible
          1 failed to access device
*************************************************************************/
unsigned char i2c_rep_start(unsigned char address);

/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void i2c_stop(void);

/*************************************************************************
  Send one byte to I2C device
  Input:    byte to be transfered
  Return:   0 write successful 
            1 write failed
*************************************************************************/
unsigned char i2c_write( unsigned char data );

/*************************************************************************
 Read one byte from the I2C device, request more data from device 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readAck(void);

 /*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readNak(void);
