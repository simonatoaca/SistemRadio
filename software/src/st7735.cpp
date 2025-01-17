#include <pins_arduino.h>
#include "spi_disp.h"
#include "binary.h"
#include "util/delay.h"
#include "font16x16.h"
#include "font8x8_basic.h"
#include "avr/pgmspace.h"
#include "string.h"

#define MIN_X ((uint8_t)0x00)
#define MAX_X ((uint8_t)0x7F)

#define MIN_Y ((uint8_t)0x00)
#define MAX_Y ((uint8_t)0x9F)

#define SW_RESET ((uint8_t)0x01)
#define DISPOFF ((uint8_t)0x28)
#define DISPON ((uint8_t)0x29)
#define CASET ((uint8_t)0x2A)
#define RASET ((uint8_t)0x2B)
#define MADCTL ((uint8_t)0x36)
#define COLMOD ((uint8_t)0x3A)
#define INVOFF ((uint8_t)0x20)
#define INVON ((uint8_t)0x21)
#define SPLOUT ((uint8_t)0x11)
#define RAMWR ((uint8_t)0x2C)
#define PTLON ((uint8_t)0x12)
#define PTLAR ((uint8_t)0x30)
#define NORON ((uint8_t)0x13)
#define IDMON ((uint8_t)0x39)
#define IDMOFF ((uint8_t)0x38)

#define LCD_RESET_PIN (PB1)
#define DC_PIN (PB0)

#define WHITE 0xffff
#define BLACK 0x0000
#define RED 0xF800

#define CHAR_HEIGHT_8 (8)
#define CHAR_WIDTH_8 (8)

#define CHAR_HEIGHT_16 (16)
#define CHAR_WIDTH_16 (16)

void lcd_send_cmd(uint8_t command)
{
	// DC low -> command
	PORTB &= ~(1 << DC_PIN);

	SPI_Write(&command, 1);
}

void lcd_send_data(uint8_t *data, size_t buf_size)
{
	// DC high -> data
	PORTB |= (1 << DC_PIN);

	SPI_Write(data, buf_size);
}

void lcd_send_16(uint16_t data)
{
	// DC high -> data
	PORTB |= (1 << DC_PIN);

	uint8_t t = data >> 8;
	SPI_Write(&t, 1);

	t = data;
	SPI_Write(&t, 1);
}

void lcd_set_position(uint16_t xs, uint16_t xe, uint16_t ys, uint16_t ye)
{
	lcd_send_cmd(RASET);

	lcd_send_16(xs);
	lcd_send_16(xe);

	lcd_send_cmd(CASET);

	lcd_send_16(ys);
	lcd_send_16(ye);
}

void lcd_set_pixel()
{
	lcd_send_cmd(RAMWR);
	lcd_send_16(RED);
}

void lcd_set_partial_area(uint16_t s, uint16_t e)
{
	lcd_send_cmd(PTLAR);

	lcd_send_16(s);
	lcd_send_16(e);

	lcd_send_cmd(PTLON);
}

void lcd_disable_partial()
{
	lcd_send_cmd(NORON);
}

void lcd_idle()
{
	lcd_send_cmd(IDMON);
}

void lcd_idle_off()
{
	lcd_send_cmd(IDMOFF);
}

void lcd_write8(const char *text, uint8_t xs, uint8_t ys, uint16_t color, uint16_t bg_color = BLACK)
{
	uint8_t len = strlen(text);

	lcd_set_position(xs, xs + CHAR_HEIGHT_8 - 1, ys, ys + (len * CHAR_WIDTH_8) - 1);

	lcd_send_cmd(RAMWR);

	for (uint8_t row = 0; row < CHAR_HEIGHT_8; row++) {
		for (uint8_t char_idx = 0; char_idx < len; char_idx++) {
			uint8_t character = pgm_read_byte(&font8x8_basic[(uint8_t)text[char_idx]][row]);

			for (uint8_t b = 0; b < 8; b++) {
				lcd_send_16(((character >> b) & 1) ? color : bg_color);
			}
		}
	}
}

void lcd_write16(const char *text, uint8_t xs, uint8_t ys, uint16_t color, uint16_t bg_color = BLACK)
{
	uint8_t len = strlen(text);

	lcd_set_position(xs, xs + CHAR_HEIGHT_16 - 1, ys, ys + (len * CHAR_WIDTH_16) - 1);

	lcd_send_cmd(RAMWR);

	for (uint8_t row = 0; row < CHAR_HEIGHT_16; row++) {
		for (uint8_t char_idx = 0; char_idx < len; char_idx++) {
			uint16_t character = (pgm_read_byte(&font16x16[(uint8_t)(text[char_idx] - 32) * CHAR_HEIGHT_16 * 2 + 2 * row]) << 8) |
							pgm_read_byte(&font16x16[(uint8_t)(text[char_idx] - 32) * CHAR_HEIGHT_16 * 2 + 2 * row + 1]);

			for (int8_t b = CHAR_WIDTH_16 - 1; b >= 0; b--) {
				lcd_send_16(((character >> b) & 1) ? color : bg_color);
			}
		}
	}
}

void lcd_write_line(uint8_t xs, uint8_t ys, uint8_t width, uint8_t height, uint16_t color)
{
	lcd_set_position(xs, xs + width - 1, ys, ys + height - 1);

	lcd_send_cmd(RAMWR);
	for (int i = 0; i < height * width; i++) {
		lcd_send_16(color);
	}
}

void lcd_fill_screen()
{
	lcd_write_line(MIN_X, MIN_Y, MAX_X, MAX_Y, BLACK);
}

void lcd_init()
{
	// Set pins as outputs
	DDRB |= (1 << DC_PIN) | (1 << LCD_RESET_PIN);

	SPI_Init();

	// Send reset
	PORTB &= ~(1 << LCD_RESET_PIN);

	_delay_ms(100);

	// Send reset
	PORTB |= (1 << LCD_RESET_PIN);

	_delay_ms(10);

	lcd_send_cmd(SW_RESET);
	_delay_ms(100);

	// Send sleep out
	lcd_send_cmd(SPLOUT);

	_delay_ms(50);

	lcd_send_cmd(DISPON);

	// Set how the data will be displayed
	lcd_send_cmd(MADCTL);
	
	uint8_t data = 0xA0;
	lcd_send_data(&data, 1);

	_delay_ms(10);

	// Set color mode 16 pixels
	lcd_send_cmd(COLMOD);

	data = 0x05;
	lcd_send_data(&data, 1);

	// Make screen black
	lcd_fill_screen();

	// Start position: 0, 0
	lcd_set_position(MIN_X, MIN_Y, MAX_X, MAX_Y);
}

void lcd_off()
{
	lcd_send_cmd(DISPOFF);
}