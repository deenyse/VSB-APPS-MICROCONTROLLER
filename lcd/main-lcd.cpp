#include "mbed.h"
#include "lcd_lib.h"
#include "fonts/font10x16_lsb.h"

#define FONT_WIDTH 10
#define FONT_HEIGHT 16
#define LCD_WIDTH 320
#define LCD_HEIGHT 240

extern uint8_t g_font8x8[256][8];

DigitalOut g_led_PTA1(PTA1, 0);
DigitalOut g_led_PTA2(PTA2, 0);

class button_control {
private:
	DigitalIn m_butt; // current is pressed state
	bool previousState = 0; // previous is pressed state
	bool isActive = 0; //changes to opposite every release

public:

	button_control(PinName button) :
			m_butt(button) {
	}
	;
	bool getIsActive() {
		return this->isActive;
	}
	;

	bool getPressStatus() {
		return this->m_butt;
	}
	;

	void usedActiveState() {
		this->isActive = 0;
	}
	void updateState() {
		if (!this->m_butt && this->previousState) { // if pressed
			this->previousState = 0;
			this->isActive = !this->isActive;

		}
		if (this->m_butt && !this->previousState) // if released
				{
			this->previousState = 1;
		}
	}
	;

};

button_control butt[4] = { PTC9, PTC10, PTC11, PTC12 };

uint16_t rgb888_to_rgb565(uint32_t rgb888_color) {
    // Extracting individual color components from RGB888 format
    uint8_t red = (rgb888_color >> 16) & 0xFF;
    uint8_t green = (rgb888_color >> 8) & 0xFF;
    uint8_t blue = rgb888_color & 0xFF;

    // Converting to RGB565 format
    uint16_t r5 = (red >> 3) & 0x1F;
    uint16_t g6 = (green >> 2) & 0x3F;
    uint16_t b5 = (blue >> 3) & 0x1F;

    // Combining into a single 16-bit value
    uint16_t rgb565_color = (r5 << 11) | (g6 << 5) | b5;

    return rgb565_color;
}

void drawChar(int charHeight, int charWidth, int x, int y,
		const uint16_t character[], uint16_t Color) {
	for (int i = 0; i < charHeight; ++i) {
		for (int j = 0; j < charWidth; ++j) {
			if ((character[i] >> (charWidth - 1 - j)) & 0x01) {
				lcd_put_pixel(x + j, y + i, Color);
			}
			//printf("%c", ((character[i] >> (7 - j)) & 0x01 ? '#' : ' '));
		}
//        printf("\n");
	}
}

void drawFont(int charHeight, int charWidth, uint16_t Color) {
	for (int i = 0; i < 256; i++) {
		drawChar(charHeight, charWidth,
				(charWidth + 1) * i % (LCD_WIDTH - charWidth),
				charHeight * ((charWidth + 1) * i / (LCD_WIDTH - charWidth)),
				font[i], Color);
	}
}

void drawRectangle(int height, int widht, int x, int y, uint16_t Color) {
	for (int i = 0; i < height; i++) {
		lcd_put_pixel(i + x, y, Color);
		lcd_put_pixel(i + x, y + widht, Color);

	}
	for (int j = 0; j < widht ; j++) {
		lcd_put_pixel(x, y + j, Color);
		lcd_put_pixel(x + height, y + j, Color);
	}
}

void drawCircle(int x0, int y0, int radius, uint16_t Color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        lcd_put_pixel(x0 + x, y0 + y, Color);
        lcd_put_pixel(x0 + y, y0 + x, Color);
        lcd_put_pixel(x0 - y, y0 + x, Color);
        lcd_put_pixel(x0 - x, y0 + y, Color);
        lcd_put_pixel(x0 - x, y0 - y, Color);
        lcd_put_pixel(x0 - y, y0 - x, Color);
        lcd_put_pixel(x0 + y, y0 - x, Color);
        lcd_put_pixel(x0 + x, y0 - y, Color);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }

        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void drawLine(int x0, int y0, int x1, int y1, uint16_t Color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx, sy;

    if (x0 < x1) sx = 1;
    else sx = -1;

    if (y0 < y1) sy = 1;
    else sy = -1;

    int err = dx - dy;
    int e2;

    while (true) {
        lcd_put_pixel(x0, y0, Color);

        if (x0 == x1 && y0 == y1) break;

        e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t Color){
	drawLine(x0,y0,x1,y1, Color);
	drawLine(x1,y1,x2,y2, Color);
	drawLine(x2,y2,x0,y0, Color);
}

int main() {
	lcd_init();				// LCD initialization

	uint16_t l_color_red = 0xF800;
	uint16_t l_color_green = 0x07E0;
	uint16_t l_color_blue = 0x001F;
	uint16_t l_color_white = 0xFFFF;


	while (1) {
//		drawFont(FONT_HEIGHT,FONT_WIDTH, l_color_red);
//		drawRectangle(100, 100, 10, 10, l_color_red);
//		drawCircle(160, 120, 120, l_color_red);
//		drawLine(10,10,120,100, l_color_red);
//		drawTriangle(10,10,30,50,70,35, rgb888_to_rgb565(0xFF0000));
		for (int i = 0; i < 4; i++) {
			butt[i].updateState();
		} // update buttons
	}
	return 0;
}
