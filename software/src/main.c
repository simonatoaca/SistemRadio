#include <pins_arduino.h>
#include "spi.h"

int main()
{
	SPI_Init();

	uint8_t data[10];

	SPI_Write(data, 10);
}