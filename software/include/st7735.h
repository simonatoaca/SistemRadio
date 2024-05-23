#ifndef __ST7735__
#define __ST7735__

#define WHITE 0xffff
#define BLACK 0x0000
#define RED 0xF800

void lcd_init();
void lcd_off();
void lcd_set_pixel();
void lcd_write_line(uint8_t xs, uint8_t ys, uint8_t width, uint8_t height, uint16_t color);
void lcd_write(const char *text, uint8_t xs, uint8_t ys, uint16_t color);
void lcd_fill_screen();
#endif