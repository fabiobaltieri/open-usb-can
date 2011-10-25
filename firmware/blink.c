#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static uint8_t led_timeout_a;
static uint8_t led_timeout_b;

void blink_init (void)
{
	TIMSK0 = ( (1 << OCIE0A) );

	TCCR0A = ( (1 << WGM01) | (0 << WGM00) ); /* CTC */
	TCCR0B = ( (0 << WGM02) |
		   (1 << CS02) | (0 << CS01) | (1 << CS00) ); /* /1024 */

	OCR0A = F_CPU / 1024 / 60; /* about 60Hz */
}

ISR(TIMER0_COMPA_vect)
{
	led_timeout_a--;
	if (led_timeout_a == 0)
		led_a_off();

	led_timeout_b--;
	if (led_timeout_b == 0)
		led_b_off();
}

void blink_tx (void)
{
	led_a_on();
	led_timeout_a = 2;
}

void blink_rx (void)
{
	led_b_on();
	led_timeout_b = 2;
}
