/* Point of entry for the pepper spray project. */

#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "camera_lib.h"
#include "bt_lib.h"

volatile unsigned char send;

/* External triggered send command */
ISR(INT2_vect){
	_delay_ms(200); // debounce.. instead of using a state machine
	if (PORTB &= 0x04) send = 1;
}

/* Set up external interrupt on INT2 for pushbutton. */
void interrupt_init(void){
	DDRB = 0x00;	// Port B as input
	PORTB = 0x04; 	// pull-up resistor set on INT2
    EIMSK = 1<<INT2 ;					// turn on int2
    EICRA = (1<<ISC21) | (1<<ISC20) ;	// rising edge
	sei();  //enable global interrupts
}

int main(void){
	int size = 0;
	DDRC = 0x10; // set C.4 to output (flash LED)
	_delay_ms(1000); // wait for power line to settle.n n      

	bt_init(); 
	camera_init();	// prepare camera unit
	bt_setSrv();


	camera_reset();
	interrupt_init();
	_delay_ms(3000); // wait for 3sec before taking the first pic...

	send = 0;
	while(1){
		if (send){
//			PORTC = 0x10;
			camera_takePic();
//			_delay_ms(3);
			camera_getSize(&size);
//			PORTC = 0x00;
			_delay_ms(1);
			bt_sendMsg();
			_delay_ms(3);
			camera_getFile();
			send = 0;
		}
	}
}
