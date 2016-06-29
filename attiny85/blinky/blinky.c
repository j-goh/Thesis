// blinky.c
//
// Blinky program for ATtiny85
// Blinks LED on pin 2 (PB3)
// One short blink and one longer
//

#define F_CPU 125000UL

#include <avr/io.h>
#include <util/delay.h>

int main (void)
{
    CLKPR = (1 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
    CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS0);
	// Set B3 to output
	DDRB = (1 << DDB4);
	while(1) {
		// Flash 1
		// Set B3 high
		PORTB = (1 << PORTB4);
		_delay_ms(200);
		
		// Set B3 low
		PORTB = (1 << PORTB4);
		_delay_ms(500);
		
		//// Flash 2
		//// Set B3 high
		//PORTB = (1 << PORTB4);
		//_delay_ms(100);
		
		//// Set B3 low
		//PORTB = (0 << PORTB4);
		//_delay_ms(100);
	}
	
	return 0;
}
	
