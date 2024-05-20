#ifndef __ST7735__
#define __ST7735__

void lcd_init();
void lcd_off();
void lcd_set_pixel();
void lcd_write_line(uint8_t xs, uint8_t ys, uint8_t len, uint8_t width, uint16_t color);
void lcd_fill_screen();
#endif