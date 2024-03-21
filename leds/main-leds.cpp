#include "mbed.h"

class ledcontrols {
private:
    Ticker m_ticker;
    Ticker b_ticker;
    DigitalOut m_led;
    bool isIncrease= true;
    float max_delay_period = 23;
    float current_delay = 10;
    int current_tick = 0;

public:
    ledcontrols(PinName led) : m_led(led) {
    	this->m_ticker.attach(callback(this, &ledcontrols::pwm), 1ms);
    	this->b_ticker.attach(callback(this, &ledcontrols::ticker_brightness),5ms);
    }

    void ticker_brightness(){
    	if (this->isIncrease){
    		this->current_delay += this->max_delay_period*0.001;
    		if (this->current_delay >= this->max_delay_period){
    			this->isIncrease = false;
    		}

    	} else {
    		this->current_delay -= this->max_delay_period*0.001;
    		if (this->current_delay <=0){
    			this->isIncrease = true;
    		}
    	}

    }
    void set_brightness(float brightness) {
    	if (brightness >1)
    		brightness = 1;
		if (brightness <0)
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

ledcontrols g_red_led[] = { { PTC1 }, { PTC0 }, { PTC7 }, { PTC3 }, { PTC4 }, { PTC5 }, { PTC2 }, { PTC8 } , {PTA1} , {PTA2} };


int main() {
    while (1) {}
}
