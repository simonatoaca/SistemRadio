#include <pins_arduino.h>
#include "util/delay.h"
#include "Arduino.h"
#include "avr/interrupt.h"

#define LEFT_TURNS B11110000
#define RIGHT_TURNS B00001111

volatile uint8_t counter = 0;
volatile uint8_t turns = 0;

void toggle_led()
{
	PORTB ^= (1 << PB5);
}

// Pin B
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

	// LED pin as output and turn it off
	DDRB |= (1 << PB5);
	PORTB &= ~(1 << PB5);

	// Enable INT1 to trigger on the rising edge
	EIMSK |= (1 << INT1);
	EICRA |= (1 << ISC11) | (1 << ISC10);


	sei();

	uint8_t old_counter = counter;
	while (1) {
		if (counter != old_counter) {
			Serial.println(counter);
			old_counter = counter;
		}
	};
}