/* ATmega1284P uart interface library.
   Channel 0 is set up communicating with PuTTY (to debug).
   Channel 1 is set up to work with a peripheral. */
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include "uart.h"

/*
 * Initialize the UART (channel = 0 or 1) to uart_baud Bd, tx/rx, 8N1.
   Set int_en to 1 if using interrupt mode, 0 if polling mode.
 */
void uart_init_0(unsigned long uart_baud, char int_en){
//	#if F_CPU < 2000000UL && defined(U2X0)
	  UCSR0A = _BV(U2X0);             /* improve baud rate error by using 2x clk */
	  UBRR0L = (F_CPU / (8UL * uart_baud)) - 1;
//	#else
//	  UBRR0L = (F_CPU / (16UL * uart_baud)) - 1;
//	UBRR0L = 8;
//	#endif
	  UCSR0B = _BV(TXEN0) | _BV(RXEN0) | (int_en << RXCIE0); 
}

void uart_init_1(unsigned long uart_baud, char int_en){
//	#if(F_CPU < 2000000UL) && #ifdef(U2X1)
	  UCSR1A = _BV(U2X1);             /* improve baud rate error by using 2x clk */
	  UBRR1L = (F_CPU / (8UL * uart_baud)) - 1;
//	#else
//	  UBRR1L = (F_CPU / (16UL * uart_baud)) - 1;
//	UBRR1L = 8;
//	#endif
	  UCSR1B = _BV(TXEN1) | _BV(RXEN1) | (int_en << RXCIE1);
}

/*
 * Send character c down the UART channel 0 or 1 Tx, wait until tx holding register
 * is empty.
 */
int uart_putchar_0(char c){
	loop_until_bit_is_set(UCSR0A, UDRE0);

	UDR0 = c;
	return 0;
}

int uart_putchar_1(char c){
	loop_until_bit_is_set(UCSR1A, UDRE1);
	UDR1 = c;
	return 0;
}

/*
 * Receive a character from the UART Rx on channel ch.
 */
char uart_getchar_0(void){
	loop_until_bit_is_set(UCSR0A, RXC0);
	if (UCSR0A & _BV(FE0)) return _FDEV_EOF;
	if (UCSR0A & _BV(DOR0)) return _FDEV_ERR;
	return UDR0;
}

char uart_getchar_1(void){
	loop_until_bit_is_set(UCSR1A, RXC1);
	if (UCSR1A & _BV(FE1)) return _FDEV_EOF;
	if (UCSR1A & _BV(DOR1)) return _FDEV_ERR;
//	uart_putchar_1(UDR1);
	return UDR1;
}

