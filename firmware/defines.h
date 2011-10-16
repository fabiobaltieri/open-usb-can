#include <stdint.h>

/* buttons */
#define BUTTON_PORT PORC
#define BUTTON_DDR  DDRC
#define BUTTON_PIN  PINC
#define BUTTON_LEFT PC4
#define BUTTON_MIDDLE PC7
#define BUTTON_RIGHT PC2

#define button_left() (BUTTON_PIN & _BV(BUTTON_LEFT))
#define button_right() (BUTTON_PIN & _BV(BUTTON_RIGHT))
#define button_middle() (BUTTON_PIN & _BV(BUTTON_MIDDLE))

/* leds */
#define LED_A_PORT PORTC
#define LED_A_DDR  DDRC
#define LED_A PC6
#define LED_B_PORT PORTC
#define LED_B_DDR DDRC
#define LED_B PC5

#define led_a_off()    LED_A_PORT |=  _BV(LED_A)
#define led_a_on()     LED_A_PORT &= ~_BV(LED_A)
#define led_a_toggle() LED_A_PORT ^=  _BV(LED_A)
#define led_b_off()    LED_B_PORT |=  _BV(LED_B)
#define led_b_on()     LED_B_PORT &= ~_BV(LED_B)
#define led_b_toggle() LED_B_PORT ^=  _BV(LED_B)

/* CAN SPI */
#define SPI_DDR  DDRB
#define SPI_PORT PORTB
#define SPI_CS   PB0
#define SPI_SCK  PB1
#define SPI_MOSI PB2
#define SPI_MISO PB3

#define can_cs_h() (CAN_CS_PORT |=  _BV(CAN_CS))
#define can_cs_l() (CAN_CS_PORT &= ~_BV(CAN_CS))
