/**
  ******************************************************************************
  * @file    attiny85/JG2016-02.c 
  * @author  Jacqueline Goh - 43238266
  * @date    20-May-2016
  * @brief   JG2016 Control main file
  ******************************************************************************
  */
  
#define F_CPU 125000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <stdint.h>

// Define set bit and clear bit macros
#define sbi(sfr,bit) (sfr |= _BV(bit))
#define cbi(sfr,bit) (sfr &= ~(_BV(bit)))

#define BUBBLE_THRESH   400
#define MAX_AIR         5


// OCR values for the 7 notes within an octave
// Calculated by ocr = timer_freq / note_freq
// Assuming a timer of 62.5kHz (to fit in 8 bits)
//const uint8_t notes[7] = {239, 213, 190, 179, 159, 142, 127};
const uint8_t notes[2] = {239, 20};

/**
 * @brief Sets the timer compare and OCR registers for timer 1 to play a tone
 *        for a specified duration. Output is on PB1
 * @param ocr - ocr value to enter into register
 * @param duration - time in ms to play the note
 * @return None
 */
void PlayTone(uint8_t ocr, uint8_t duration)
{
    TCCR1 = (1 << CTC1) | (1 << COM1A0) | (1 << CS11); // Start OC w. prescaler of 2
    OCR1C = ocr; // set the OCR
    _delay_ms(duration);
    TCCR1 = (1 << CTC1) | (1 << COM1A0); // stop the counter
}

/**
 * @brief Loops through the seven notes in "notes" and plays each for 1 ms
 * @param None
 * @return None
 */
void PlayAlarm(void)
{
    uint8_t i = 0;
    for(i = 0; i < 2; i++) {
        PlayTone(notes[i], 1);
    }
}

/**
  * @brief  ADC initialisation. Sets the ADMUX and ADCSRA registers.
  * @return Nothing
  */
void ADCInit (void)
{
    /* Clear ADEN bit of register - halts all ADC processes */
    cbi(ADCSRA, ADEN);
	/* Setup ADMUX register */
	ADMUX = (0 << REFS2) | (1 << REFS1) | (0 << REFS0); /* Set reference voltage to 1V1 */
	/* Select PB2 (ADC1) with single ended input */
	ADMUX |= (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0); 
	
	ADCSRA |= (0 << ADATE); /* Auto trigger disabled */
	/*
	 * Recommended ADC clock is between 50kHz and 200kHz
	 * If using internal 8MHz clock, prescaler must be 128 or 64
     * If using 125kHz clock, prescaler must be 2 --> 62.5kHz
	 */
	ADCSRA |= (0 << ADPS2) | (0 << ADPS1) | (1 << ADPS0); /* Prescaler of 2 */

    /* Enable ADC. Ref and channel selections will not go into effect until set */
    sbi(ADCSRA, ADEN);
}   

/**
 * @brief Retrieves the the 10bit value from the analog input on PB4
 * @return 10 bit digital value
 */
uint16_t ADCGet (void)
{
    uint8_t high, low;
    sbi(ADCSRA, ADSC); /* Start conversion */
    while(bit_is_set(ADCSRA, ADSC)); /* Wait for conversion to finish */

    /* Lower byte must be read first */
    low = ADCL;
    high = ADCH;

    return (high << 8) | low;
}

/**
  * @brief  Blinks LED once for ~200ms. Toggles the B0 GPIO Pin.
  * @return Nothing
  */
void BlinkLED (void)
{
	/* Set B0 high */
    sbi(PORTB, PORTB0);
	_delay_ms(200);
	
	/* Set B0 low */
    cbi(PORTB, PORTB0);
	_delay_ms(100);
}

int main (void)
{
    uint16_t on_value, off_value;
    uint32_t stored = 0;
	/* Set clock prescaler to 64 (125kHz clock speed) */
	CLKPR = (1 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
	CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS0);    
	
	/* Set B4, B1 and B0 to output */
	DDRB = (1 << DDB4) | (1 << DDB1) | (1 << DDB0);
	ADCInit();
	sei(); /* Enable interrupts */
	
	while(1) {
        /* Keep alive LED on B0 */
        //BlinkLED();

        /* TODO: turn on and off ADC, and have appropriate delay between */
        /* ADC retreival */
		sbi(PORTB, PORTB4);
        _delay_ms(100);
        on_value = ADCGet();
        
        cbi(PORTB, PORTB4);
        _delay_ms(100);
        off_value = ADCGet();

        /* Check if a bubble was detected */
        if((on_value - off_value) < BUBBLE_THRESH) {
            /* Increment stored value */
            sbi(PORTB, PORTB0);
            if (++stored > MAX_AIR) {
                /* Maximum amount of air has entered line */
                //while(1) {
                    //PlayAlarm();
                //}
                PlayAlarm();
                stored = 0; /* Reset stored air amount DEBUG */
            }
        } else {
            cbi(PORTB, PORTB0);
        }
	}
	
	return 0;
}

//~ /**
  //~ * @brief  ADC Converstion ISR. Runs once an ADC conversion is complete
  //~ * @param ADC_vect - ADC vector as specified for ATtiny85
  //~ * @return Nothing
  //~ */
//~ ISR(ADC_vect)
//~ {
	//~ uint8_t detected;
	//~ detected = ADCH;
	//~ // Set B0 high if ADC is more than reference level
	//~ if(detected > ref) {
		//~ ref = detected;
		//~ PORTB |= (1 << PORTB0);
		//~ _delay_ms(200);

	//~ } else {
		//~ //ref--;
		//~ PORTB &= ~(1 << PORTB0);
	//~ }
	
	//~ ADCSRA |= (1 << ADSC); // Restart the conversion
//~ }
	
