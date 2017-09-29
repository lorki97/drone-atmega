/*
 * UART0.h
 *
 * Created: 29.09.2017 20:07:13
 *  Author: flola
 */ 


#ifndef UART0_H_
#define UART0_H_

#include <avr/io.h>
#include "config.h"
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>

typedef void (*UART_RECV_CALLBACK)(uint8_t recivedChar);

/*
 *	Sets all required registers for UART communication
 *
 */
void uart0_init(uint32_t BaudRate);

/*
 *	returns true if there is space in the send buffer
 *
 */
bool uart0_buffer_has_space();

/*
 *	Add a char to the UART send buffer
 * IMPORTANT: Check if the buffer has space before calling
 *			  this function. Otherwise the char will be discarded
 *			  if the buffer is full.
 */
void uart0_putc(uint8_t character);

/*
 *	Register a callback function which gets called when a char is received
 */
void uart0_register_recived_callback(UART_RECV_CALLBACK callBack);


#endif /* UART0_H_ */