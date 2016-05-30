/**
  ******************************************************************************
  * @file    attiny85/JG2016-02.c 
  * @author  Jacqueline Goh - 43238266
  * @date    20-May-2016
  * @brief   JG2016 Control main file
  ******************************************************************************
  */
  
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>


volatile uint8_t ref = 0;

/**
  * @brief  ADC initialisation. Sets the ADMUX and ADCSRA registers.
  * @return Nothing
  */
void ADCInit (void)
{
	/* Setup ADMUX register */
	ADMUX = (0 << REFS2) | (1 << REFS1) | (0 << REFS0); /* Set reference voltage to internal 1.1V */
	ADMUX |= (1 << ADLAR); /* Left adjust bits (8 bit mode) */
	/* Select PB3 (ADC3) with single ended input */
	ADMUX |= (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0); 
	
	ADCSRA = (1 << ADEN); /* Enable ADC */
    ADCSRA |= (0 << ADATE); /* Auto trigger disabled */
    ADCSRA |= (1 << ADIE); /* Interrupt enabled */
	/*
     * Recommended ADC clock is between 50kHz and 200kHz
     *If using internal 8MHz clock, prescaler must be 128 or 64
     */
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); /* Prescaler of 128 */
}	

/**
  * @brief  Blinks LED. Toggles the B4 GPIO Pin.
  * @return Nothing
  */
void BlinkLED (void)
{
    /* Set B3 high */
    PORTB |= (1 << PORTB4);
    _delay_ms(200);
    
    /* Set B3 low */
    PORTB &= ~(1 << PORTB4);
    _delay_ms(200);
}
    

int main (void)
{
    /* Set clock prescaler to 8 (1MHz clock speed) */
    //CLKPR = (1 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
    //CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (1 << CLKPS1) | (1 << CLKPS0);    
    
    /* Set B4 and B0 to output */
	DDRB = (1 << DDB4) | (1 << DDB0);
    ADCInit();
    sei(); /* Enable interrupts */
    ADCSRA |= (1 << ADSC);
    
	while(1) {
        BlinkLED();
	}
	
	return 0;
}

/**
  * @brief  ADC Converstion ISR. Runs once an ADC conversion is complete
  * @param ADC_vect - ADC vector as specified for ATtiny85
  * @return Nothing
  */
ISR(ADC_vect)
{
    uint8_t detected;
    detected = ADCH;
    // Set B0 high if ADC is more than reference level
    if(detected > ref) {
        ref = detected;
        PORTB |= (1 << PORTB0);
        _delay_ms(200);

    } else {
        //ref--;
        PORTB &= ~(1 << PORTB0);
    }
    
    ADCSRA |= (1 << ADSC); // Restart the conversion
}
    
