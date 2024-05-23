#include <pins_arduino.h>
#include "binary.h"
#include "util/delay.h"
#include "Arduino.h"
#include "avr/interrupt.h"
#include "st7735.h"
#include "RDA5807.h"

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

// char *programInfo;
// char *stationName;
// char *rdsTime;

// long delayStationName = millis();
// long delayProgramInfo = millis();
// uint8_t idxProgramInfo = 0;

volatile uint16_t freq = 10670; // Default: Europa FM 106.7 MHz

RDA5807 rx;

void radio_init()
{
	/* TODO: Retrieve default freq from EEPROM */

	rx.setup();

  	rx.setRDS(true);
  	rx.setRdsFifo(true);

  	rx.setVolume(6);
  	rx.setMono(false);
  	rx.setMute(false);

  	rx.setFrequency(freq);   // It is the frequency you want to select in MHz multiplied by 100.
  	rx.setFmDeemphasis(1);   // Sets to 50 μs. Used in Europe, Australia, Japan.
  	rx.setSeekThreshold(50); // Sets RSSI Seek Threshold (0 to 127)
  	rx.setAFC(true);
}

// SW of Rotary Encoder
ISR(INT0_vect) {
	/* TODO: Save current radio station to EEPROM */
}

// Pin B of Rotary Encoder
ISR(INT1_vect) {
	// Pin A was already activated -> right turn
	if (PIND & (1 << PD4)) {
		turns++;
		freq += 10 * ((turns & RIGHT_TURNS) >> 1);
		turns -= 2 * ((turns & RIGHT_TURNS) >> 1);
	} else {
		turns += (1 << 4);
		freq -= 10 * (((turns & LEFT_TURNS) >> 4) >> 1);
		turns -= (2 * (((turns & LEFT_TURNS) >> 4) >> 1) << 4);
	}
}

void setup()
{
	// Serial.begin(9600);

	// _delay_ms(200);

	// LED pin as output and turn it off
	DDRB |= (1 << PB5);
	PORTB &= ~(1 << PB5);

	// rotary_encoder_init();

	radio_init();
	lcd_init();

	sei();

	_delay_ms(10);

	lcd_write("Hello, world!", 10, 10, WHITE);
}

void loop()
{
	// uint32_t old_freq = freq;

	// while (1) {
	// 	if (old_freq != freq) {
	// 		Serial.println(freq);
	// 		old_freq = freq;
	// 	}
	// }
}