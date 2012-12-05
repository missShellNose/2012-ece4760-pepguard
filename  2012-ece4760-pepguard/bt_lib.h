#ifndef BT_LIB_H
#define BT_LIB_H

#include "uart.h"

#define BLOCKSIZE 32 // size of a block of JPEG data to read. must be multiples of 8.

unsigned char BT_RESPONSE[50];

// UART file descriptor
// putchar_1 and getchar_1 are in uart.c; using channel 1 for stdio 
//extern FILE uart_str; 

/* Initialize camera. Return 1 if successful, 0 if failed. */
void bt_init(void);

/* Read the size of the JPEG file. 2-byte response stored in argument.
   Return 1 if successful, 0 if failed. */
void bt_getInfo(void);
void bt_sendPic(void);
void bt_setSrv(void);

void bt_sendMsg(void);
/* Send 1-byte chunk of picture data through bluetooth. */
void bt_sendPicture(char c);

#endif
