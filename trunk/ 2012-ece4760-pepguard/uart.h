#ifndef UART_H
#define UART_H

/* CPU frequency */
#define F_CPU 16000000UL
/* size of the receive buffer for channel 0 (PuTTY)*/
#define RX_BUFSIZE 80

/* initialize channel 0, used to connect to PuTTY for debugging.*/
void uart_init_0(unsigned long baud0, char int_en0);

/* initialize channel 1, used to connect to the camera. */
void uart_init_1(unsigned long baud1, char int_en1);

int uart_putchar_0(char c);

int uart_putchar_1(char c);

char uart_getchar_0(void);

char uart_getchar_1(void);

#endif
