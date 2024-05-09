#ifndef __SPI_DRIVER__
#define __SPI_DRIVER__

void SPI_Init();
void SPI_Write(uint8_t *buffer, size_t buf_size);

#endif /* __SPI_DRIVER__ */
