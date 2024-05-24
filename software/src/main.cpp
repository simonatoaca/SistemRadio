#include <pins_arduino.h>
#include "binary.h"
#include "util/delay.h"
#include "Arduino.h"
#include "avr/interrupt.h"
#include "avr/sleep.h"
#include "util/atomic.h"
#include "st7735.h"
#include "actions.h"
#include "RDA5807.h"

#define MIN_FREQ ((uint16_t)9000)
#define MAX_FREQ ((uint16_t)12000)

#define FREQ_CHANGE ((uint8_t)0x00)
#define CHAN_SELECT_ENTER ((uint8_t)0xff)

#define DUMMY (0)
#define UP (1)
#define DOWN (2)
#define SELECT (3)

#define N_CHANNELS 4

typedef struct {
	uint16_t freq;
	char *station;
} chan_t;

RDA5807 radio;

volatile uint16_t freq = 10060;  // Default: RockFM
volatile uint16_t old_freq = freq;

volatile uint8_t action_type = FREQ_CHANGE;
volatile action current_action = actions[DUMMY];
volatile int8_t current_idx = 0; // Index for channel menu
volatile int8_t old_current_idx = -1;
volatile uint32_t lastScreenUpdate = millis();

chan_t saved_channels[N_CHANNELS] = {
	{
		.freq = 10670,
		.station = "EuropaFM"
	},
	{
		.freq = 10060,
		.station = "RockFM  "
	},
	{
		.freq = 9610,
		.station = "KissFM  "
	},
	{
		.freq = 9080,
		.station = "MagicFM "
	}
};

void print_freq(uint16_t freq)
{
	char text[6];
	sprintf(text, "%d.%d", freq / 100, (freq % 100) / 10);

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		lcd_write16(text, 10, 10, WHITE);
	}
}

void print_menu()
{
	if (old_current_idx == -1) {
		lcd_fill_screen();
	}

	if (millis() - lastScreenUpdate > 200 && current_idx != old_current_idx) {
		uint8_t start_idx = current_idx < 3 ? 0 : (current_idx % 3 + current_idx / 3);
		start_idx = min(start_idx, N_CHANNELS - 3);

		for (uint8_t i = start_idx; i < min(start_idx + 3, N_CHANNELS); i++) {
			char text[7];
			sprintf(text, "%d.%d ", saved_channels[i].freq / 100, (saved_channels[i].freq % 100) / 10);
			text[5] = 0;

			uint16_t bg_color = (i == current_idx) ? YELLOW : WHITE;

			// Print frequency and radio station
			lcd_write16(text, 10 + 38 * (i - start_idx), 10, BLACK, bg_color);
			lcd_write8(saved_channels[i].station, 10 + 38 * (i - start_idx) + 18, 10, BLACK, bg_color);
		}

		lastScreenUpdate = millis();
		old_current_idx = current_idx;

		// current_action = actions[DUMMY];
	}
}

void update_freq()
{
	if (millis() - lastScreenUpdate > 200 && freq != old_freq) {
		radio.setFrequency(freq);
		print_freq(freq);

		lastScreenUpdate = millis();
		old_freq = freq;

		// current_action = actions[DUMMY];
	}
}

void dummy()
{
	set_sleep_mode(SLEEP_MODE_ADC);
	sleep_enable();
}

void select_chan()
{
	if (action_type == CHAN_SELECT_ENTER) {
		// Enter select menu
		current_action = print_menu;
	} else {
		lcd_fill_screen();
		lcd_write16(" MHz", 10, 5 * 16 + 10, WHITE);

		// Update freq
		current_action = update_freq;
	}
}

// char *programInfo;
// char *stationName;
// char *rdsTime;

// long delayStationName = millis();
// long delayProgramInfo = millis();
// uint8_t idxProgramInfo = 0;

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

void radio_init()
{
	/* TODO: Retrieve default freq from EEPROM */

	radio.setup();

  	// radio.setRDS(true);
  	// radio.setRdsFifo(true);

  	radio.setVolume(5);
  	radio.setMono(false);
  	radio.setMute(false);
	radio.setBass(false);

  	radio.setFrequency(freq);   // It is the frequency you want to select in MHz multiplied by 100.
  	radio.setFmDeemphasis(1);   // Sets to 50 μs. Used in Europe, Australia, Japan.
  	radio.setSeekThreshold(50); // Sets RSSI Seek Threshold (0 to 127)
  	radio.setAFC(true);
}

// SW of Rotary Encoder
ISR(INT0_vect) {
	// Enter channel menu / Exit channel menu
	action_type ^= 0xff;

	// Reinitialize values
	current_idx = 0;
	old_freq = 0;
	old_current_idx = -1;

	current_action = actions[SELECT];
}

// Pin B of Rotary Encoder
ISR(INT1_vect) {
	// Pin A was already activated -> right turn
	if (PIND & (1 << PD4)) {
		if (action_type == FREQ_CHANGE) {
			freq += 10;

			current_action = update_freq;
		} else {
			current_idx++;
			current_idx = min(current_idx, N_CHANNELS - 1);

			current_action = print_menu;
		}
	} else {
		if (action_type == FREQ_CHANGE) {
			freq -= 10;

			current_action = update_freq;
		} else {
			current_idx--;
			current_idx = max(current_idx, 0);

			current_action = print_menu;
		}
	}
}

void setup()
{
	// Serial.begin(9600);

	// _delay_ms(200);

	// LED pin as output and turn it off to save power
	DDRB |= (1 << PB5);
	PORTB &= ~(1 << PB5);

	rotary_encoder_init();
	radio_init();
	lcd_init();
	lcd_idle();

	// Disable a lot of things for power saving:
	ADCSRA = 0;

	// PRR |= (1 << PRTIM0) | (1 << PRTIM1) | (1 << PRTIM2);

	sei();

	print_freq(freq);

	/* This is written only once */
	lcd_write16(" MHz", 10, 5 * 16 + 10, WHITE);
}

void loop()
{
	current_action();
}