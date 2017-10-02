/*
 * UARTCOM.h
 *
 * Created: 29.09.2017 23:25:38
 *  Author: flola
 */ 


#ifndef UARTCOM_H_
#define UARTCOM_H_

#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include "config.h"
#include "UART0.h"

#define ACK_CHAR	6	//Acknowledge
#define NACK_CHAR	21	//Negative acknowledge
#define START_CHAR	2	//Start of text
#define STOP_CHAR	4	//End of transmission

struct transmissionData  
{
	uint8_t Type;
	uint8_t Length;
	uint8_t* Data;
	uint8_t CRC[4];
}; 
typedef struct transmissionData transmissionData;


#define LENGTH(x)  (sizeof(x) / sizeof((x)[0]))

/*
* Initializes everything needet for the UARTCOMM Module
* the interrupts have to be enabled afterwards manually
*/
void UARTCOM_init(uint32_t BaudRate);


/*
* Checks if there is enought space in the send buffer to send the required data
* Suggested implementation (LENGTH only works if the dataArray is NOT passed by a pointer!): 
* ...
* uint8_t dataArray[] = { ... };
* if( UARTCOMM_ready_to_send(LENGTH(dataArray)) )
* {
* 	UARTCOM_transmit_block( ... );
* }
*/
bool UARTCOM_ready_to_send(transmissionData Data);

/*
* Sends data over the UARTCOMM Module after a safety check.
* Returns the data structure if sucessful, returns NULL if unsucsessful
* Suggested implementation (LENGTH only works if the dataArray is NOT passed by a pointer!): 
* uint8_t dataArray[] = { ... };
* UARTCOM_transmit_block(0-256, dataArray, LENGTH(dataArray));
*/
transmissionData UARTCOM_transmit_block(uint8_t Type, const uint8_t Data[], uint8_t Length);


#endif /* UARTCOM_H_ */