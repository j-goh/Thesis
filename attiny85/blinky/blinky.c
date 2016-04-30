// main.c
//
// Blinky program for ATtiny85
// Blinks LED on pin 2 (PB3)
// One short blink and one longer
//

#include <avr/io.h>
#include <util/delay.h>

int main (void)
{
	// Set B3 to output
	DDRB = 0x08;
	while(1) {
		// Flash 1
		// Set B3 high
		PORTB = 0x08;
		_delay_ms(100);
		
		// Set B3 low
		PORTB = 0x00;
		_delay_ms(200);
		
		// Flash 2
		// Set B3 high
		PORTB = 0x08;
		_delay_ms(200);
		
		// Set B3 low
		PORTB = 0x00;
		_delay_ms(200);
	}
	
	return 1;
}
	