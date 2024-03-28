#include "mbed.h"

class ledcontrols {
private:

	DigitalOut m_led;
	float max_delay_period = 26;
	float current_delay = 10;
	int current_tick = 0;
	float current_brightness = 0.5;
	float eventIncreaser = 1;
public:
	ledcontrols(PinName led) :
			m_led(led) {

	}

	void set_brightness(float brightness) {
		if (brightness > 1)
			brightness = 0.05;
		if (brightness < 0.05)
			brightness = 1;
		this->current_brightness = brightness;
		this->current_delay = (this->max_delay_period * (1 - brightness));
	}

	void turnOff() {
		this->current_delay = max_delay_period;
	}
	void turnOn() {
		this->set_brightness(this->current_brightness);
	}
	void startEvent() {
		this->eventIncreaser = 4;
	}

	void finishEvent() {
		this->eventIncreaser = 1;
	}

	void pwm() {
		if (this->current_tick <= this->current_delay * this->eventIncreaser)
			this->m_led.write(0);
		else
			this->m_led.write(1);

		this->current_tick++;
		if (this->current_tick
				> this->max_delay_period * this->eventIncreaser) {
			this->current_tick = 0;
		}
	}

	float getBrightness() {
		return this->current_brightness;
	}
};

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
		}
		if (this->m_butt && !this->previousState) // if released
				{
			this->previousState = 1;
			this->isActive = !this->isActive;
		}
	}
	;

};

//DigitalOut rgb_leds[3][3] = { { PTB2, PTB3, PTB9 }, { PTB19, PTB20, PTB23 }, {PTB10, PTB11, PTB18 } };
DigitalOut status_leds[] = {{PTA1},{PTA2}};

ledcontrols rgb_light_led[] = { { PTB18 }, { PTB23 }, { PTB9 } };

ledcontrols g_red_led[] = { { PTC0 }, { PTC1 }, { PTC2 }, { PTC3 }, { PTC4 }, {
		PTC5 }, { PTC7 }, { PTC8 } };

button_control butt[4] = { PTC9, PTC10, PTC11, PTC12 };

void allLedsControl() {
	for (int i = 0; i < 8; i++) {
		g_red_led[i].pwm();
	}
}

void allRgbLightsControl() {
	for (int i = 0; i < 3; i++) {
		rgb_light_led[i].pwm();
	}
}
int main() {


	Ticker m_ticker;
	m_ticker.attach(callback(allLedsControl), 1ms);

	Ticker r_ticker;
	r_ticker.attach(callback(allRgbLightsControl), 1ms);


	for (int i = 0; i < 3; i++) {
		rgb_light_led[i].turnOff();
	}

	bool isLed = 0;

	int currentLed = 0;
	int currentRgb = 0;

	while (1) {

		for (int i = 0; i < 4; i++) {
			butt[i].updateState();
		}

		if (!butt[0].getPressStatus() && !butt[1].getPressStatus()) {
			if (isLed)
				for (int i = 0; i < 8; i++)
					g_red_led[i].set_brightness(0.5);
			else
				for (int i = 0; i < 3; i++)
					rgb_light_led[i].set_brightness(0.5);

			butt[0].usedActiveState();
			butt[1].usedActiveState();
		} else if (butt[0].getIsActive()) {
			if (isLed) {
				currentLed++;
				if (currentLed > 7)
					currentLed = 0;
			} else {
				currentRgb++;
				if (currentRgb > 2)
					currentRgb = 0;
			}
			butt[0].usedActiveState();
		} else if (butt[1].getIsActive()) {
			if (isLed)
				g_red_led[currentLed].set_brightness(
						g_red_led[currentLed].getBrightness() + 0.1);
			else
				rgb_light_led[currentRgb].set_brightness(
						rgb_light_led[currentRgb].getBrightness() + 0.1);
			butt[1].usedActiveState();
		} else if (butt[3].getIsActive()) {
			if (isLed) {
				for (int i = 0; i < 8; i++)
					g_red_led[i].turnOff();
				for (int i = 0; i < 3; i++)
					rgb_light_led[i].turnOn();
				status_leds[1] = 1;
			} else {
				for (int i = 0; i < 3; i++)
					rgb_light_led[i].turnOff();
				for (int i = 0; i < 8; i++)
					g_red_led[i].turnOn();
				status_leds[1] = 0;
			}
			isLed = !isLed;
			butt[3].usedActiveState();
		} else if (butt[2].getIsActive()) {
			if (isLed)
			for (int i = 0; i < 8; i++)
				g_red_led[i].startEvent();
			else for (int i = 0; i < 3; i++)
				rgb_light_led[i].startEvent();
			status_leds[0].write(1);
		} else if (!butt[2].getIsActive()) {
			if (isLed)
			for (int i = 0; i < 8; i++)
				g_red_led[i].finishEvent();
			else for (int i = 0; i < 3; i++)
				rgb_light_led[i].finishEvent();
			status_leds[0].write(0);
		}

	}

}
