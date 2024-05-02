#include <mbed.h>

#include "i2c-lib.h"
#include "si4735-lib.h"
// Direction of I2C communication
#define R	0b00000001
#define W	0b00000000

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

DigitalOut g_led_PTA1(PTA1, 0);
DigitalOut g_led_PTA2(PTA2, 0);

DigitalIn butt[4] = { PTC9, PTC10, PTC11, PTC12 };

class EinRadio {
private:
	uint8_t volume = 45;
	const uint8_t min_volume = 0;
	const uint8_t max_volume = 63;
	int freq = 10480;
public:
	EinRadio() {
		this->setVolume();
		this->setFreq();
		this->readFreq();
		printf("... station tuned.\r\n\n");
	}
	void volumeUp() {
		this->volume += 9;

		if (volume > 63)
			volume = 0;

		this->setVolume();
	}
	void volumeDown() {
		this->volume -= 9;

		if (volume < 9)
			volume = 63;

		this->setVolume();
	}
	void setVolume() {
		//	uint8_t S1, S2, RSSI, SNR, MULT, CAP;
		//	int freq3;

		i2c_start();
		i2c_output( SI4735_ADDRESS | W);
		i2c_output(0x12);
		i2c_output(0x00);
		i2c_output(0x40);
		i2c_output(0x00);
		i2c_output(0x00);
		i2c_output(this->volume);
		i2c_stop();
	}

	void setFreq() {
		i2c_start();
		i2c_output( SI4735_ADDRESS | W);
		i2c_output(0x20);			// FM_TUNE_FREQ
		i2c_output(0x00);			// ARG1
		i2c_output(this->freq >> 8);		// ARG2 - FreqHi
		i2c_output(this->freq & 0xff);	// ARG3 - FreqLo
		i2c_output(0x00);			// ARG4
		i2c_stop();
		printf("Current seted frequency: %d.%dMHz\r\n", this->freq / 100,
				this->freq % 100);
	}

	void lookUp() {
		this->freq += 10;
		this->setFreq();
	}

	void lookDown() {
		this->freq -= 10;
		this->setFreq();
	}

	void seekUp() {
		int ack = 0;
		i2c_start();
		ack |= i2c_output( SI4735_ADDRESS | W);
		ack |= i2c_output(0x21);
		ack |= i2c_output(0b00001100);
		i2c_stop();
		wait_us(100000);
		this->readFreq();
		printf("seekUp worked\n");

	}

	void seekDown() {
		int ack = 0;
		i2c_start();
		ack |= i2c_output( SI4735_ADDRESS | W);
		ack |= i2c_output(0x21);
		ack |= i2c_output(0b00000100);
		i2c_stop();

		wait_us(1000000);
		this->readFreq();
		printf("seekDown worked\n");
	}

	int readFreq() {
		uint8_t S1, S2, RSSI, SNR, MULT, CAP;
		int freq3;
		i2c_start();
		i2c_output( SI4735_ADDRESS | W);
		i2c_output(0x22);			// FM_TUNE_STATUS
		i2c_output(0x00);
		i2c_start();
		// change direction of communication
		i2c_output( SI4735_ADDRESS | R);
		// read data
		S1 = i2c_input();
		i2c_ack();
		S2 = i2c_input();
		i2c_ack();
		freq3 = (int) i2c_input() << 8;
		i2c_ack();
		freq3 |= i2c_input();
		i2c_ack();
		RSSI = i2c_input();
		i2c_ack();
		SNR = i2c_input();
		i2c_ack();
		MULT = i2c_input();
		i2c_ack();
		CAP = i2c_input();
		i2c_nack();
		i2c_stop();
		this->freq = freq3;
		this->setFreq();
		printf("Current frequency: %d.%dMHz\n", this->freq / 100, this->freq % 100);
		printf("Main data: RSSI: %d, SNR: %d, MULT: %d, CAP: %d\n", RSSI, SNR,
				MULT, CAP);
	}
	;
};

class panelHandler {
private:
	uint8_t swAdress = 0b01000000;
public:
	void leds_on(int numBits) {
		uint8_t ack = 0;
		i2c_start();
		ack |= i2c_output(this->swAdress);
		ack |= i2c_output((1 << numBits) - 1);
		i2c_stop();
	}

	void onAnimation() {
		for (int i = 1; i <= 8; i++) {
			leds_on(i);
			wait_us(200000);
		}
		for (int i = 7; i >= 0; i--) {
			leds_on(i);
			wait_us(200000);
		}
	}

};

bool isReleased[4] = { 1, 1, 1, 1 };

int main(void) {
	printf("K64F-KIT ready...\r\n");
	i2c_init();
	if (si4735_init() != 0) {
		printf("Initialization of SI4735 finish with error\n");
		return 0;
	} else
		printf("SI4735 initialized.\r\n");

	panelHandler myPanel;
	EinRadio myRadio;
	wait_us(100000);
	for (int i = 0; i < 3; i++)
		myPanel.onAnimation();

	bool isReleased[4] = { 1, 1, 1, 1 };
	uint32_t buttonTimer[4] = { 0, 0, 0, 0 };
	bool validButtonTimer[4] = { 0, 0, 0, 0 };
	while (1) {
		for (int i = 0; i < 4; i++) {
			if (!butt[i].read() && isReleased[i]) {
				if (i == 0) {
					myRadio.volumeDown();
				}
				if (i == 1) {
					myRadio.volumeUp();
				}
				if (i == 2) {
					if (validButtonTimer[2])
						myRadio.seekDown();
					else
						myRadio.lookDown();
				}
				if (i == 3) {
					if (validButtonTimer[3])
						myRadio.seekUp();
					else
						myRadio.lookUp();
				}
				isReleased[i] = false;
				if (!validButtonTimer[i]) {
					validButtonTimer[i] = 1;
				} else {
					buttonTimer[i] = 0;
					validButtonTimer[i] = 0;
				}

			} else if (butt[i].read() && !isReleased[i]) {
				isReleased[i] = true;
			}

			if (validButtonTimer[i]) {
				buttonTimer[i]++;
				if (buttonTimer[i] > 16000) {
					buttonTimer[i] = 0;
					validButtonTimer[i] = 0;
				}
			}
		}

	}
	return 0;
}
