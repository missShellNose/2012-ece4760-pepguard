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
int uart_putchar_0(char c, FILE *stream){
  if (c == '\a')
    {
      fputs("*ring*\n", stderr);
      return 0;
    }

  if (c == '\n')
    uart_putchar_0('\r', stream);
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;

  return 0;
}

int uart_putchar_pic(char c){
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
int uart_getchar_0(FILE *stream){
  uint8_t c;
  char *cp, *cp2;
  static char b[RX_BUFSIZE];
  static char *rxp;

  if (rxp == 0)
    for (cp = b;;)
      {
	loop_until_bit_is_set(UCSR0A, RXC0);
	if (UCSR0A & _BV(FE0))
	  return _FDEV_EOF;
	if (UCSR0A & _BV(DOR0))
	  return _FDEV_ERR;
	c = UDR0;
	/* behaviour similar to Unix stty ICRNL */
	if (c == '\r')
	  c = '\n';
	if (c == '\n')
	  {
	    *cp = c;
	    uart_putchar_0(c, stream);
	    rxp = b;
	    break;
	  }
	else if (c == '\t')
	  c = ' ';

	if ((c >= (uint8_t)' ' && c <= (uint8_t)'\x7e') ||
	    c >= (uint8_t)'\xa0')
	  {
	    if (cp == b + RX_BUFSIZE - 1)
	      uart_putchar_0('\a', stream);
	    else
	      {
		*cp++ = c;
		uart_putchar_0(c, stream);
	      }
	    continue;
	  }

	switch (c)
	  {
	  case 'c' & 0x1f:
	    return -1;

	  case '\b':
	  case '\x7f':
	    if (cp > b)
	      {
		uart_putchar_0('\b', stream);
		uart_putchar_0(' ', stream);
		uart_putchar_0('\b', stream);
		cp--;
	      }
	    break;

	  case 'r' & 0x1f:
	    uart_putchar_0('\r', stream);
	    for (cp2 = b; cp2 < cp; cp2++)
	      uart_putchar_0(*cp2, stream);
	    break;

	  case 'u' & 0x1f:
	    while (cp > b)
	      {
		uart_putchar_0('\b', stream);
		uart_putchar_0(' ', stream);
		uart_putchar_0('\b', stream);
		cp--;
	      }
	    break;

	  case 'w' & 0x1f:
	    while (cp > b && cp[-1] != ' ')
	      {
		uart_putchar_0('\b', stream);
		uart_putchar_0(' ', stream);
		uart_putchar_0('\b', stream);
		cp--;
	      }
	    break;
	  }
      }

  c = *rxp++;
  if (c == '\n')
    rxp = 0;

  return c;
}

char uart_getchar_1(void){
	loop_until_bit_is_set(UCSR1A, RXC1);
	if (UCSR1A & _BV(FE1)) return _FDEV_EOF;
	if (UCSR1A & _BV(DOR1)) return _FDEV_ERR;
	return UDR1;
}

