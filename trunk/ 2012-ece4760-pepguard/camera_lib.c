/* Library contains functions for interfacing with LinkSprite camera (TTL)
   as specified in the datasheet. */
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "camera_lib.h"
#include "bt_lib.h"

//Commands for the LinkSprite Serial JPEG Camera
const unsigned char GET_SIZE[5] = {0x56, 0x00, 0x34, 0x01, 0x00};
const unsigned char GS_Response[7] = {0x76, 0x00, 0x34, 0x00, 0x04, 0x00, 0x00};
const unsigned char RESET_CAMERA[4] = {0x56, 0x00, 0x26, 0x00};
const unsigned char RS_Response[4] = {0x76, 0x00, 0x26, 0x00};
const unsigned char TAKE_PICTURE[5] = {0x56, 0x00, 0x36, 0x01, 0x00};
const unsigned char TP_Response[5] = {0x76, 0x00, 0x36, 0x00, 0x00};
const unsigned char STOP_TAKING_PICS[5] = {0x56, 0x00, 0x36, 0x01, 0x03};
const unsigned char ST_Response[5] = {0x76, 0x00, 0x36, 0x00, 0x00};
const unsigned char READ_DATA[8] = {0x56, 0x00, 0x32, 0x0C, 0x00, 0x0A, 0x00, 0x00};
const unsigned char RD_Response[5] = {0x76, 0x00, 0x32, 0x00, 0x00};

// UART file descriptor
// putchar_1 and getchar_1 are in uart.c; using channel 1 for stdio 
//FILE uart_str = FDEV_SETUP_STREAM(uart_putchar_0, uart_getchar_0, _FDEV_SETUP_RW);

/* Initialize camera. Return 1 if successful, 0 if failed. */
int camera_init(void){
	// channel 1 UART at 38400 baud, not using interrupt 
	// channel 0 UART at 9600 baud without interrupt to echo to computer
//	uart_init_0(9600, 0);
	uart_init_1(38400, 0);
	return 1;
}

/* Send camera command (cmd) that has (l1) bytes 
	and store response of (l2) bytes at (resp). */
void camera_sendCommand(const unsigned char *cmd, unsigned char *resp, int l1, int l2){
	int i;
//	const unsigned char *ptr_cmd = cmd;
	unsigned char dummy;
//	fprintf(stdout, "\n\r *******Commands*******\n\r");
	// display command on uart_1
//	for (i=0; i<l1; i++)
//		fprintf(stdout, "| %x ", *ptr_cmd++);
//	fprintf(stdout,"\n\r*****Response*****\n\r");

	// send command on Tx
	for (i=0; i<l1; i++)
		uart_putchar_1(*cmd++);
	while ( UCSR1A & (1<<RXC1) ) dummy = UDR1;
	// receive response on Rx
	for (i=0; i<l2; i++)
		*resp++=uart_getchar_1();	
	
	// display response on uart_1
//	for (i=0; i<l2; i++)
//		fprintf(stdout, "| %x", *ptr_resp++);
//	fprintf(stdout, "\n\r\n\r");
}

/* Read the size of the JPEG file. 2-byte response stored in argument.
   Return 1 if successful, 0 if failed. */
int camera_getSize(unsigned int *response){
	unsigned char *resp_ptr = CAM_RESPONSE;
	unsigned char *ref_ptr = GS_Response;
	int i;
	camera_sendCommand(GET_SIZE, CAM_RESPONSE, 5, 9);
	for (i = 0; i < 7; i++)
		if (*resp_ptr++ != *ref_ptr++) return 0;
	*response = (unsigned int) (((*resp_ptr) << 8) | (*(resp_ptr+1)));
	return 1;
}

/* Reset the camera. Return 1 if successful, 0 if failed. */
char camera_reset(void){
	unsigned char *resp_ptr = CAM_RESPONSE;
	unsigned char *ref_ptr = RS_Response;
	int i;
	camera_sendCommand(RESET_CAMERA, CAM_RESPONSE, 4, 4);
	for (i = 0; i < 4; i++)
		if (*resp_ptr++ != *ref_ptr++) return 0;
	return 1;
}

/* Take a picture. Return 1 if successful, 0 if failed. */
char camera_takePic(void){
	unsigned char *resp_ptr = CAM_RESPONSE;
	unsigned char *ref_ptr = TP_Response;
	int i;
	PORTC = 0x10;
	_delay_ms(80);
	PORTC = 0x00;
	camera_sendCommand(TAKE_PICTURE, CAM_RESPONSE, 5, 5);
	for (i = 0; i < 5; i++)
		if (*resp_ptr++ != *ref_ptr++) return 0;
	return 1;
}

/* Read a block(BLOCKSIZE) of data starting at location (addr)
   and store return value in (response). 
   Return 1 on success, 0 on failure. */
int camera_getBlock(char *response, int addr){
	int i;
	unsigned char *ptr = READ_DATA;
	unsigned char *resp_ptr = CAM_RESPONSE; 
	unsigned char *ref_ptr = RD_Response;
	// send command to get block of data
	for (i = 0; i < 8; i++) 
		uart_putchar_1(*ptr++);
	uart_putchar_1(addr >> 8); // starting address of block
	uart_putchar_1((char)addr);
	uart_putchar_1(0x00);
	uart_putchar_1(0x00);
	uart_putchar_1(BLOCKSIZE >> 8); // block size
	uart_putchar_1((char)BLOCKSIZE);
	uart_putchar_1(0x00); // spacing interval
	uart_putchar_1(0x0A);

	// get the response header (76, 0, 32, 0, 0)
	for (i = 0; i < 5; i++){
		*resp_ptr=uart_getchar_1();
		if (*resp_ptr++ != *ref_ptr++)	return 0;
	}
	
	i = 0; 
	// get the actual block of data
	while (i < BLOCKSIZE){
		*response++ = uart_getchar_1();
		i++;
	}
	// get the terminating string (76, 0, 32, 0, 0) 
	for (i = 0; i < 5; i++) uart_getchar_1();
	return 1;
}

/* Get the stored JPEG file and send it through bluetooth.
   Return 1 if successful, 0 if failed. */
char camera_getFile(void){
	int i, addr = 0; // read blocks starting from addr = 0.
	unsigned int size;
	char resp[BLOCKSIZE];
	if (!camera_getSize(&size)) return 0;

	while (addr < 150000){
		if (!camera_getBlock(resp, addr)) return 0;
		for (i=0; i<BLOCKSIZE; i++){
			bt_sendPicture(resp[i]);
			if ((resp[i] == 0xD9) && (resp[i-1] == 0xFF)) return 1;
		}
		addr += BLOCKSIZE;
	}
	return 1;
}
