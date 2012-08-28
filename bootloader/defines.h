#include <stdint.h>

/* buttons */
#define BUTTON_PORT PORTD
#define BUTTON_DDR  DDRD
#define BUTTON_PIN  PIND
#define BUTTON_HWB PC7

#define button_read() (BUTTON_PIN & _BV(BUTTON_HWB))

#define condition_init() { BUTTON_PORT |= _BV(BUTTON_HWB); _delay_us(10); }
#define condition_check() (button_read())

/* leds */
#define LED_A_PORT PORTC
#define LED_A_DDR  DDRC
#define LED_A PC5
#define LED_B_PORT PORTC
#define LED_B_DDR DDRC
#define LED_B PC4

#define led_init() { LED_A_DDR  |=  _BV(LED_A); LED_B_DDR  |=  _BV(LED_B); }

#define led_a_on()     LED_A_PORT |=  _BV(LED_A)
#define led_a_off()    LED_A_PORT &= ~_BV(LED_A)
#define led_a_toggle() LED_A_PORT ^=  _BV(LED_A)
#define led_b_on()     LED_B_PORT |=  _BV(LED_B)
#define led_b_off()    LED_B_PORT &= ~_BV(LED_B)
#define led_b_toggle() LED_B_PORT ^=  _BV(LED_B)

#define led_on  led_a_on
#define led_off led_a_off

/* CAN SPI */
#define SPI_DDR  DDRB
#define SPI_PORT PORTB
#define SPI_SS   PB0
#define SPI_SCK  PB1
#define SPI_MOSI PB2
#define SPI_MISO PB3

#define can_cs_h() (SPI_PORT |=  _BV(SPI_SS))
#define can_cs_l() (SPI_PORT &= ~_BV(SPI_SS))

/* MCP2515 defines */
#define INSTRUCTION_WRITE       0x02
#define CANCTRL       0x0f
