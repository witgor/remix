/* Acquire ADC samples using a timer with polling for available samples */

#include <pd.h>
#include <adc.h>
#include <term.h>
#include <stdio.h>

/* a few globals */
int i;
int timer = 0;
int adcchannels[4] = {0, 1, 2, 3};
int adcsmoothing[4] = {4, 64, 32, 16};
unsigned int tsample;

/* setup adc and start sampling */
void adc_setup(void) {
	for (i = 0; i < 4; i++) {
		adc_setblocking(adcchannels[i], 0);
		adc_setsmoothing(adcchannels[i], adcsmoothing[i]);
		/* get 4 samples per second, per channel */
		adc_setclock(adcchannels[i], 4, timer);
	}
	delete timer;
}

/* draw static text on terminal */
void draw_text(void) {
	term_clrscr();
	term_puts(1, 1, "ADC Status:");
	term_puts(1, 3, "CH  SLEN  RES");
	term_puts(1, 9, "Press ESC to exit.");
}

/* start sampling on all channels at the same time */
void start_sampling(void) {
	for (i = 0; i < 4; i++)
		adc_sample(adcchannels[i], 128);
}

int handle_kbd(int k) {
	if (k == term_decode("KC_ESC"))
		return 1;
	return 0;
}

int main(void) {
	int key;
	adc_setup();
	draw_text();
	char temp[5];
	while (1) {
		for (i = 0; i < 4; i++) {
			if (adc_isdone(adcchannels[i]) == 1)
				adc_sample(adcchannels[i], 128);
			tsample = adc_getsample(adcchannels[i]);
			if (! tsample == 0) {
				sprintf(temp, "%d", adcchannels[i]);
				term_puts(1, i + 4, temp);
				sprintf(temp, "%d", adcsmoothing[i]);
				term_puts(5, i + 4, temp);
				sprintf(temp, "%d", tsample);
				term_puts(11, i + 4, temp);
			}
		}
		key = term_getchar(NOWAIT);
		if (key != -1) {
			if (handle_kbd((int)key))
				break;
		}
	}
	term_clrscr();
	term_moveto(1, 1);
	return 0;
}
