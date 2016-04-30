// adc.c
//
// Example ADC program for ATtiny85
// on pin 2 (PB3)
// Blinks LED if ADC is greater than 512
//
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

void ADCInit (void)
{
	// Setup ADMUX register
	ADMUX = (0 << REFS1) | (0 << REFS0); // Set reference voltage to Vcc
	ADMUX |= (1 << ADLAR); // Right adjust bits
	// Select PB4 (ADC2) with single ended input
	ADMUX |= (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0); 
	
	ADCSRA = (1 << ADEN); // Enable ADC
    ADCSRA |= (0 << ADATE); // Auto trigger disabled
    ADCSRA |= (1 << ADIE); // Interrupt enabled
	// Recommended ADC clock is between 50kHz and 200kHz
	// If using internal 8MHz clock, prescaler must be 128 or 64
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler of 128
}	

void BlinkLED (void)
{
    // Set B3 high
    PORTB |= (1 << PORTB3);
    _delay_ms(200);
    
    // Set B3 low
    PORTB &= ~(1 << PORTB3);
    _delay_ms(200);
}
    

int main (void)
{
    
    // Set B3 to output
	DDRB = (1 << DDB3) | (1 << DDB0);
    ADCInit();
    sei();
    ADCSRA |= (1 << ADSC); 
	while(1) {
        BlinkLED();
	}
	
	return 0;
}

ISR(ADC_vect)
{
    if(ADCH > 128) {
        PORTB |= (1 << PORTB0);
    } else {
        PORTB &= ~(1 << PORTB0);
    }
    
    ADCSRA |= (1 << ADSC);
}
    