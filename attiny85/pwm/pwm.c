// pwm.c
//
// Tone generator for ATtiny85
// Outputs a square wave on pin B1
//

#define F_CPU 125000UL

#include <avr/io.h>
#include <util/delay.h>

// OCR values for the 7 notes within an octave
// Calculated by ocr = timer_freq / note_freq
// Assuming a timer of 62.5kHz (to fit in 8 bits)
const uint8_t notes[7] = {239, 213, 190, 179, 159, 142, 127};

/**
 * @brief Sets the timer compare and OCR registers for timer 1 to play a tone
 *        for a specified duration
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
void PlayScale(void)
{
    uint8_t i = 0;
    for(i = 0; i < 7; i++) {
        PlayTone(notes[i], 1);
    }
}

int main(void)
{
    // Enable output
    DDRB = (1 << DDB1);
    // Change clock prescaler to 64 (F_CPU = 125kHz)
    CLKPR = (1 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
    CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS0);
    while(1) {
        play_scale();
        _delay_ms(10);
    }
}
