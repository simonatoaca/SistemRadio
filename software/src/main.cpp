#include <pins_arduino.h>
#include "binary.h"
#include "util/delay.h"
#include "Arduino.h"
#include "avr/interrupt.h"
#include "st7735.h"

float p = 3.1415926;

#define LEFT_TURNS B11110000
#define RIGHT_TURNS B00001111

volatile uint8_t counter = 0;
volatile uint8_t turns = 0;


void rotary_encoder_init()
{
	// Set SW pin (PD2) as input and enable pull-up resistor
	PORTD |= (1 << PD2);

	// Enable:
	// INT0 to trigger on the falling edge
	// INT1 to trigger on the rising edge
	EIMSK |= (1 << INT1) | (1 << INT0);
	EICRA |= (1 << ISC11) | (1 << ISC10) | (1 << ISC01);
}

// SW of Rotary Encoder
ISR(INT0_vect) {
	// counter++;
}

// Pin B of Rotary Encoder
ISR(INT1_vect) {
	// Pin A was already activated -> right turn
	if (PIND & (1 << PD4)) {
		turns++;
		counter += (turns & RIGHT_TURNS) >> 1;
		turns -= 2 * ((turns & RIGHT_TURNS) >> 1);
	} else {
		turns += (1 << 4);
		counter -= ((turns & LEFT_TURNS) >> 4) >> 1;
		turns -= (2 * (((turns & LEFT_TURNS) >> 4) >> 1) << 4);
	}
}

int main()
{
	Serial.begin(9600);

	_delay_ms(500);

	// LED pin as output and turn it off
	DDRB |= (1 << PB5);
	PORTB &= ~(1 << PB5);

	// rotary_encoder_init();

	sei();

	lcd_init();

	_delay_ms(10);

	// lcd_write_line(50, 50, 15, 5, 0xf800);
	lcd_fill_screen();
}