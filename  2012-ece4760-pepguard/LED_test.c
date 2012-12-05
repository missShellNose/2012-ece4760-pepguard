#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>

int main(void){
	DDRC = 0x10; // set C.4 to output
	while(1){
		_delay_ms(3000);
//		PORTC = 0x10;
		_delay_ms(80);
		PORTC = 0x00;
		
	}
	return 0;
}
