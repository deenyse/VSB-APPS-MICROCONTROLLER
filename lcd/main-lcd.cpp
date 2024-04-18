#include "mbed.h"
#include "lcd_lib.h"
#include "fonts/font10x16_lsb.h"
#include "fonts/font10x16_msb.h"

#define FONT_WIDTH 10
#define FONT_HEIGHT 16
#define LCD_WIDTH 320
#define LCD_HEIGHT 240

DigitalOut g_led_PTA1(PTA1, 0);
DigitalOut g_led_PTA2(PTA2, 0);

DigitalIn butt[4] = { PTC9, PTC10, PTC11, PTC12 };

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
			if ((character[i] >> (j)) & 0x01) {
				lcd_put_pixel(x + j, y + i, Color);
			}
			//printf("%c", ((character[i] >> (7 - j)) & 0x01 ? '#' : ' '));
		}
//        printf("\n");
	}
}

int numDigits(int number) {
	int digits = 1;
	while (number /= 10) {
		digits++;
	}
	return digits;
}

void drawDigits(int score, int charHeight, int charWidth, int x, int y,
		const uint16_t font_lsb[][FONT_HEIGHT], uint16_t Color) {
	int num = score;
	int digits = numDigits(score);

	// Draw each digit
	for (int i = digits - 1; i >= 0; i--) {
		int digit = num % 10;
		drawChar(charHeight, charWidth, x + i * charWidth, y,
				font_lsb[48 + digit], Color); // Assuming font contains digits from '0' to '9'
		num /= 10;
	}
}

void draw_score(int l_score, int r_score, uint16_t Color) {
	int charHeight = FONT_HEIGHT; // Assuming FONT_HEIGHT is defined somewhere
	int charWidth = FONT_WIDTH;   // Assuming FONT_WIDTH is defined somewhere
//    const uint16_t font_lsb[][FONT_HEIGHT] = {
//        // Font data
//    };

	// Calculate the total width required for both scores
	int totalWidth = charWidth * ((numDigits(l_score) + 1) + numDigits(r_score))
			+ 1; // +1 for spacing

	// Draw left score digits
	drawDigits(l_score, charHeight, charWidth, (LCD_WIDTH - totalWidth) / 2, 0,
			font_lsb, Color);

	// Draw right score digits
	drawDigits(r_score, charHeight, charWidth,
			(LCD_WIDTH - totalWidth) / 2 + charWidth * (numDigits(l_score) + 1)
					+ 1, 0, font_lsb, Color);
}

void drawFont(int charHeight, int charWidth, uint16_t Color,
		const uint16_t font_lsb[][FONT_HEIGHT]) {
	for (int i = 0; i < 256; i++) {
		drawChar(charHeight, charWidth,
				(charWidth + 1) * i % (LCD_WIDTH - charWidth),
				charHeight * ((charWidth + 1) * i / (LCD_WIDTH - charWidth)),
				font_lsb[i], Color);
	}
}

void drawRectangle(int height, int width, int x, int y, uint16_t Color) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			lcd_put_pixel(x + i, y + j, Color);
		}
	}
}

class panel {
private:
	int height = 45;
	int width = 5;
	int y = 0;
	int x;
	int speed = 4;
public:
	panel(int x) {
		this->x = x;
		this->y = (LCD_HEIGHT - this->height) / 2;
		drawRectangle(this->width, this->height, this->x, this->y, 0xFFFF);
	}

	int get_y() {
		return this->y;
	}

	int get_width() {
		return this->width;
	}

	int get_height() {
		return this->height;
	}

	void mooveUp() {
		if ((this->y - this->speed) >= 0) {
			y -= speed;
			drawRectangle(this->width, this->speed, this->x, this->y, 0xFFFF);
			drawRectangle(this->width, this->speed, this->x,
					this->y + this->height, 0x0000);

		} else
			y = 0;
	}

	void mooveDown() {
		if ((this->y + this->speed) <= (LCD_HEIGHT - this->height)) {
			y += speed;
			drawRectangle(this->width, this->speed, this->x,
					this->y + this->height - this->speed, 0xFFFF);
			drawRectangle(this->width, this->speed, this->x,
					this->y - this->speed, 0x0000);
		}

		else
			y = (LCD_HEIGHT - this->height);
	}
};

class ball {
private:
	int x = LCD_WIDTH / 2;
	int y = LCD_HEIGHT / 2;
	int speed = 6;
	int dx = speed;
	int dy = -speed;
	int radius = 5;
public:

	int moove(int l_platform_y, int r_platform_y, int platform_height,
			int platform_width) { // int 0 - nohing; int 1 - left scored; int 2 - right scored
		drawRectangle(this->radius, this->radius, this->x, this->y, 0x0000);
		this->x += this->dx;
		this->y += this->dy;

		if (this->y < 0) {
			this->y = 0;
			this->dy *= -1;
		}
		if (this->y > (LCD_HEIGHT - this->radius)) {
			this->y = (LCD_HEIGHT - this->radius);
			this->dy *= -1;
		}

		if (this->dx < 0) {
			if (this->x <= platform_width + this->speed + 2) {
				if ((this->y + this->radius) < l_platform_y
						|| this->y > (l_platform_y + platform_height)) {
					this->x = LCD_WIDTH / 2;
					this->y = LCD_HEIGHT / 2;
					this->dx *= -1;

					return 2;
				} else {
					this->dx *= -1;
				}
			}
		}

		if (this->dx > 0) {
			if ((this->x + this->radius)
					>= (LCD_WIDTH - platform_width - this->speed - 2)) {
				if ((this->y + this->radius) < r_platform_y
						|| this->y > (r_platform_y + platform_height)) {
					this->x = LCD_WIDTH / 2;
					this->y = LCD_HEIGHT / 2;
					this->dx *= -1;

					return 1;
				} else {
					this->dx *= -1;
				}
			}
		}
		drawRectangle(this->radius, this->radius, this->x, this->y, 0xFFFF);
	}

	int get_y() {
		return this->y;
	}
};
int main() {
	lcd_init();
	panel left_panel(0);
	panel right_panel(LCD_WIDTH - 10);
	ball ball;

	int l_count = 0;
	int r_count = 0;

	while (1) {

		if (!butt[0].read()) {
			left_panel.mooveUp();
		} else if (!butt[1].read()) {
			left_panel.mooveDown();
		}


		int move_res = ball.moove(left_panel.get_y(), right_panel.get_y(),
				right_panel.get_height(), right_panel.get_width());

		if (ball.get_y() > right_panel.get_y() + (right_panel.get_height()/2)) {
			right_panel.mooveDown();
		} else {
			right_panel.mooveUp();
		}


		if (move_res != 0) {
			draw_score(l_count, r_count, 0x0000);
			if (move_res == 1) {
				l_count++;
			}
			if (move_res == 2) {
				r_count++;
			}
			draw_score(l_count, r_count, 0xFFFF);
		}
	}

	return 0;
}
