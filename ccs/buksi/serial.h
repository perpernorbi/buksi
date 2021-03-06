/*
 * serial.h
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#ifndef SERIAL_H_
#define SERIAL_H_


void serial_initialize();
void serial_receiveByte(char data);
const char * serial_getNextFrame();
void serial_sendChar(char data);
const char * serial_getNextCharToSend();
void serial_sendString(const char* data);
void serial_test();
#endif /* SERIAL_H_ */
