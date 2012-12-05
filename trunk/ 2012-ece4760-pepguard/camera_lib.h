#ifndef CAMERA_LIB_H
#define CAMERA_LIB_H

#include "uart.h"

#define BLOCKSIZE 32 // size of a block of JPEG data to read. must be multiples of 8.

unsigned char CAM_RESPONSE[10];

// UART file descriptor
// putchar_1 and getchar_1 are in uart.c; using channel 1 for stdio 
extern FILE uart_str; 

/* Initialize camera. Return 1 if successful, 0 if failed. */
int camera_init(void);

/* Read the size of the JPEG file. 2-byte response stored in argument.
   Return 1 if successful, 0 if failed. */
int camera_getSize(unsigned int *response);

/* Reset the camera. Return 1 if successful, 0 if failed. */
char camera_reset(void);

/* Take a picture. Return 1 if successful, 0 if failed. */
char camera_takePic(void);

/* Get the stored JPEG file and store it in microSD card.
   Must be called after SD card is initialized... 
   Return 1 if successful, 0 if failed. */
char camera_getFile(void);

#endif
