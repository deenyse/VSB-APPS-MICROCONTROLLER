#include "mbed.h"
class ledcontrols {
private:

	DigitalOut m_led;
	float max_delay_period = 26;
	float current_delay = 10;
	int current_tick = 0;
	float current_brightness = 0.5;

public:
	ledcontrols(PinName led) :
			m_led(led) {
	}

	void set_brightness(float brightness) {
		if (brightness > 1)
			brightness = 1;
		if (brightness < 0)
			brightness = 0;
		this->current_brightness = brightness;
		this->current_delay = (this->max_delay_period * (1 - brightness));
	}

	void turnOff() {
		this->set_brightness(0);
	}

	void pwm() {
		if (this->current_tick <= this->current_delay)
			this->m_led.write(0);
		else
			this->m_led.write(1);

		this->current_tick++;
		if (this->current_tick > this->max_delay_period) {
			this->current_tick = 0;
		}
	}

	void addBrightness() {
		this->current_brightness += 0.1;
		this->set_brightness(this->current_brightness);
	}

	void decreaseBrightness() {
		this->current_brightness -= 0.1;
		this->set_brightness(this->current_brightness);
	}

	float getBrightness() {
		return this->current_brightness;
	}
};



//DigitalOut rgb_leds[3][3] = { { PTB2, PTB3, PTB9 }, { PTB19, PTB20, PTB23 }, {PTB10, PTB11, PTB18 } };

ledcontrols rgb_light_led[] = { { PTB2 }, { PTB3 }, { PTB9 }, { PTB10 },
		{ PTB11 }, { PTB18 }, { PTB19 }, { PTB20 }, { PTB23 } };

DigitalOut g_red_led[] = { { PTC0 }, { PTC1 }, { PTC2 }, { PTC3 }, { PTC4 }, {
		PTC5 }, { PTC7 }, { PTC8 } };

button_control butt[4] = { PTC9, PTC10, PTC11, PTC12 };

void allRgbLightsControl() {
	for (int i = 0; i < 9; i++) {
		rgb_light_led[i].pwm();
	}
}

int main() {
	Ticker r_ticker;
	r_ticker.attach(callback(allRgbLightsControl), 1ms);

	int currentLed = 0;

	while (1) {

		for (int i = 0; i < 4; i++) {
			butt[i].updateState();
		} // update buttons

		for (int i = 0; i < 8; i++) {
			g_red_led[i] = 0;
		}

		for (int i = 0;
				i < round(rgb_light_led[currentLed * 3].getBrightness() * 8);
				i++) {
			g_red_led[i] = 1;
		}

		if (!butt[0].getPressStatus() && !butt[1].getPressStatus()) {
			for (int i = 0; i < 9; i++)
				rgb_light_led[i].turnOff();

			butt[0].usedActiveState();
			butt[1].usedActiveState();

		} else if (butt[0].getIsActive()) {
			currentLed++;
			if (currentLed > 2)
				currentLed = 0;
			butt[0].usedActiveState();
		} else if (butt[1].getIsActive()) {
			rgb_light_led[currentLed * 3].decreaseBrightness();
			rgb_light_led[currentLed * 3 + 1].decreaseBrightness();
			rgb_light_led[currentLed * 3 + 2].decreaseBrightness();
			butt[1].usedActiveState();
		} else if (butt[2].getIsActive()) {
			rgb_light_led[currentLed * 3].addBrightness();
			rgb_light_led[currentLed * 3 + 1].addBrightness();
			rgb_light_led[currentLed * 3 + 2].addBrightness();
			butt[2].usedActiveState();

		} else if (butt[3].getIsActive()) {
			if (currentLed <= 1) {
				rgb_light_led[(currentLed + 1) * 3].set_brightness(
						rgb_light_led[currentLed * 3].getBrightness());
				rgb_light_led[(currentLed + 1) * 3 + 1].set_brightness(
						rgb_light_led[currentLed * 3].getBrightness());
				rgb_light_led[(currentLed + 1) * 3 + 2].set_brightness(
						rgb_light_led[currentLed * 3].getBrightness());
			} else {
				rgb_light_led[0].set_brightness(
						rgb_light_led[currentLed * 3].getBrightness());
				rgb_light_led[1].set_brightness(
						rgb_light_led[currentLed * 3].getBrightness());
				rgb_light_led[2].set_brightness(
						rgb_light_led[currentLed * 3].getBrightness());

			}

			butt[3].usedActiveState();
		}

	}

}
