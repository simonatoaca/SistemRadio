#include <pins_arduino.h>
#include "spi.h"

/* PAGINA 176 din datasheet */

void SPI_Init()
{
	/* Set MOSI, SCK and CS as outputs */
	DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5);

	/* Config: Master Mode, MSB first, SDA sampled on the rising edge */
	SPCR |= (1 << MSTR);
	SPCR &= ~((1 << DORD) | (1 << CPOL) | (1 << CPHA));

	/* Set SPI frequency to 8MHz */
	SPCR &= ~((1 << SPR1) | (1 << SPR0));
	SPCR |= (1 << SPI2X);

	/* Enable SPI */
	SPCR |= (1 << SPE);
}

static inline void SPI_WriteByte(uint8_t data)
{
	SPDR = (uint8_t)data;

	/* Wait for transaction to complete */
	while (!(SPSR & (1 << SPIF)));
}

void SPI_Write(uint8_t *buffer, size_t buf_size)
{
	/* Set CS to low */
	PORTB &= ~(1 << PB2);

	for (size_t i = 0; i < buf_size; i++) {
		SPI_WriteByte(buffer[i]);
	}

	/* Set CS to high */
	PORTB |= (1 << PB2);
}