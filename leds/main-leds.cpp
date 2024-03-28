#include "mbed.h"

class ledcontrols {
private:
	Ticker m_ticker;
	Ticker b_ticker;
	DigitalOut m_led;
	float max_delay_period = 25;
	float current_delay = 10;
	int current_tick = 0;

public:
	ledcontrols(PinName led) :
			m_led(led) {
		this->m_ticker.attach(callback(this, &ledcontrols::pwm), 1ms);
	}

	void set_brightness(float brightness) {
		if (brightness > 1)
			brightness = 1;
		if (brightness < 0)
			brightness = 0;
		this->current_delay = (max_delay_period * (1 - brightness));
	}

	void pwm() {
		if (this->current_tick < this->current_delay)
			this->m_led.write(0);
		else
			this->m_led.write(1);

		this->current_tick++;
		if (this->current_tick >= this->max_delay_period) {
			this->current_tick = 0;
		}
	}

};

class button_control {
private:
	DigitalIn m_butt;
	bool previousState = 0;
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

ledcontrols g_red_led[] = { { PTC1 }, { PTC0 }, { PTC7 }, { PTC3 }, { PTC4 }, {
		PTC5 }, { PTC2 }, { PTC8 } };

button_control butt[4] = { PTC9, PTC10, PTC11, PTC12 };

int main() {
	while (1) {
		butt[1].updateState();
		if (butt[1].getIsActive()) {
			for (int i = 0; i < 8; i++) {
				g_red_led[i].set_brightness(0.05);
			}

		} else {
			for (int i = 0; i < 8; i++) {
				g_red_led[i].set_brightness(1);
			}
		}
	}
}
