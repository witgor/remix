
/*
 * eLua morse demo; Dado Sutter, sep 2008
 * Adapted for remix by Raman Gopalan
 */

#include <pio.h>
#include <tmr.h>
#include <pwm.h>
#include <term.h>
#include <pd.h>


/* globals */
unsigned int dot_delay = 90000, play_freq = 880, play_freq_save = 880,
freq_step = 220, dot_delay_step = 10000;
int pwmid = 1, tmrid = 1;

/* symbols */
char *morse[26] = { 
		    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....",
		    "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.",
		    "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-",
		    "-.--", "--.."
};

int morse_index(char x)
{
	return (int)x - 65;
}

void play(char m)
{
	if (m == ' ') {
		tmr_delay(tmrid, 2 * dot_delay);
	} else {
		pio_pin_sethigh("PF_0");
		pwm_start(pwmid);
		tmr_delay(tmrid, m == '.' ? dot_delay : 3 *dot_delay);
		pwm_stop(pwmid);
		pio_pin_setlow("PF_0");
		tmr_delay(tmrid, dot_delay);
	}
}
	
/* kayboard handler */
int handle_kbd(int k)
{
	if (k == term_decode("KC_ESC")) {
		return 1;
	} else if (k == term_decode("KC_UP")) {
		dot_delay += dot_delay_step;
	} else if (k == term_decode("KC_DOWN")) {
		dot_delay -= dot_delay_step;
	} else if (k > 0 && k < 256) {
		if (k == '+') {
			play_freq += freq_step;
		} else if (k == '-') {
			play_freq += freq_step;
		} else if (k == 's') {
			if (play_freq == 0) {
			        play_freq = play_freq_save;
			} else {
        			play_freq_save = play_freq;
        			play_freq = 0;
			}
		}
	}
  	pwm_setup(pwmid, play_freq, 50);
	return 0;
}
				
/* get started */
void get_started(void) {
	pio_pin_setdir(OUTPUT, "PF_0");
	pwm_setup(pwmid, play_freq, 50);
	play(' ');
	play('.');
	play('-');
}

/* the magic starts here */
int main(void)
{
	char msg[20];
	int key;
	int index, enabled = 1, i, j;
	if (strcmp("EK-LM3S8962", pd_board()) != 0 || strcmp("EK-LM3S6965", pd_board() != 0)) {
		printf("%s is not supported with this example\n");
		return -1;
	}

	get_started();

	while (1) {
		term_clrscr();
		term_moveto(1, 1);
		printf("Welcome to picoc Morse Playing on %s", pd_board());
		printf("\nEnter phrase (empty phrase to exit): ");
		gets(msg);
		if (strlen(msg) == 0)
			break;
		while (term_getchar(0) != -1);
		while (enabled) {
			for (i = 0; msg[i]; i++) {
				index = morse_index(msg[i]);
				printf ("%c", msg[i]);
				printf (": ");
				if (msg[i] != ' ') {
					for (j = 0; morse[index][j]; j++) {
						printf ("%c", morse[index][j]);
						play(morse[index][j]);
					}
					printf ("\n");
				} else {
					play(' ');
					play(' ');
				}
				play(' ');
				key = term_getchar(0);
				if (key != -1) {
					if (handle_kbd((int)key)) {
						enabled = 0;
						exit(1);
					}
				}
			}
			if (!enabled)
				break;
			printf("\n\n");
			play(' ');
			play(' ');
			play(' ');
		}
	}
	return 0;
}
