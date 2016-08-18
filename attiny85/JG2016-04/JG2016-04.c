/**
  ******************************************************************************
  * @file    attiny85/JG2016-03.c 
  * @author  Jacqueline Goh - 43238266
  * @date    1-Aug-2016
  * @brief   JG2016-03 Control main file
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


#define SAMPLE_RATE     20
#define SAMPLE_PERIOD   (1000/SAMPLE_RATE)
#define BUBBLE_THRESH   8
/* Maximum amount of air to enter a line
 * Depending on the sample rate, each positive sample indicates a volume of air
 * air volume = flow_rate * tube_diamter * sample_period / 2
 * e.g. sampling at 20Hz w 50% duty cycle = 50ms on time
 * 50ms @ flow rate of 10mm/s w. tube size of 2.5mm radius (20mm^2) = 10mm^3 = 0.01cm
 * Spec = 300mL, so we would need 3000 bubbles
 */
#define MAX_AIR         100
#define FILT_LENGTH     8


// OCR values for the 7 notes within an octave
// Calculated by ocr = timer_freq / note_freq
// Assuming a timer of 62.5kHz (to fit in 8 bits)
//const uint8_t notes[7] = {239, 213, 190, 179, 159, 142, 127};
const uint8_t notes[2] = {239, 20};

// Interrupt counter for switch
volatile uint8_t timer_flag;

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
  * @brief  Interrupt initialisation. Sets the GIMSK and PCMSK registers.
  * @return Nothing
  */
void IntInit (void)
{
    /* Enable external interrupts */
    sbi(GIMSK, PCIE);
    /* Enable PCINT3 */
    sbi(PCMSK, PCINT3);
    /* Enable interrupts */
    sei();
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

/**
  * @brief Calculates the output of a moving average filter.
  * @param old_sample - oldest sample from the filter, new_sample - newest
  * sample from the filter, total - sum of FILT_LENGTH samples
  * @return output from filter (uint16_t)
  */
uint16_t AvgFilt(uint16_t *samples, uint16_t new_sample)
{
    uint16_t total = 0;
    uint8_t i;
    
    for(i = 0; i < FILT_LENGTH; i++) {
        /* If we reach the end of the samples, replace with new_sample */
        if(i == 7) { 
            samples[i] = new_sample;
        }
        /* Update each sample with the next newest */
        samples[i] = samples[i+1];
        /* Add the newly written samples */
        total += samples[i];
    }
    
    return (total / FILT_LENGTH);
}

int main (void)
{
    uint8_t i = 0;
    uint16_t water_value, on_value;
    uint16_t bubble_count = 0;
    /* Set clock prescaler to 64 (125kHz clock speed) */
    CLKPR = (1 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
    CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS0);    
    
    /* Set B4, B1 and B0 to output */
    DDRB = (1 << DDB4) | (1 << DDB1) | (1 << DDB0);
    ADCInit();
    IntInit();

    /* Indicate that calibration is required */
    timer_flag = 1;

    while(1) {
        /* React to switch turn on */
        if(timer_flag) {
            /* Turn on indicator LED for initialisation */
            sbi(PORTB, PORTB0);
            sbi(PORTB, PORTB4);
            _delay_ms(100);
            /* Initilise water value */
            water_value = ADCGet();
            _delay_ms(100);;
            cbi(PORTB, PORTB4);
#ifdef AVG_FILT
            /* Fill first few samples */
            for(i = 0; i<FILT_LENGTH; i++) {
                on_value = AvgFilt(samples, ADCGet());
            }
#endif
            cbi(PORTB, PORTB0);
            PlayAlarm();
            i = 0;
            timer_flag = 0;
        }

        /* TODO: turn on and off ADC, and have appropriate delay between */
        /* ADC retreival */
        _delay_ms(SAMPLE_PERIOD / 2); /* Off time */
        sbi(PORTB, PORTB4); /* Turn on sensor LED */
#ifdef AVG_FILT
        on_value = AvgFilt(samples, ADCGet());
#endif
        _delay_ms(SAMPLE_PERIOD / 4); /* On time / 2 */
        on_value = ADCGet();
        _delay_ms(SAMPLE_PERIOD / 4); /* On time / 2 */

        i++;
        cbi(PORTB, PORTB4); /* Turn off sensor LED */

        /* Check if a bubble was detected */
        if(on_value < (BUBBLE_THRESH*water_value/10)){
            /* Bubble detected */
            if(++bubble_count > MAX_AIR) {
                /* Sound alarm indefinitely */
                sbi(PORTB, PORTB0);
                while(1) {
                    PlayAlarm();
                }
            }
            sbi(PORTB, PORTB0);
        } else if (i == 200) {
            /* Recalculate water_value every 10 seconds */
            sbi(PORTB, PORTB4);
            _delay_ms(50);
            /* Initilise water value */
            water_value = ADCGet();
            _delay_ms(50);;
            cbi(PORTB, PORTB4);
            i = 0;
        } else {
            cbi(PORTB, PORTB0);
        }

    }
    return 0;
}

/**
 * @brief PCINT0 external pin interrupt on pin B3
 * @param PCINT0_vect - External interrupt vector
 * @return Nothing
 */
ISR(PCINT0_vect)
{
    timer_flag = 1;
}

