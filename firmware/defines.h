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

/* wheel */
#define WHEEL_PORT PORTB
#define WHEEL_DDR  DDRB
#define WHEEL_PIN  PINB
#define WHEEL_A PB5
#define WHEEL_B PB6

#define WHEEL_MASK (_BV(WHEEL_A) | _BV(WHEEL_B))
#define WHEEL_SHIFT 5

#define WHEEL_PCMSK    PCMSK0
#define WHEEL_A_PCINT  PCINT5
#define WHEEL_B_PCINT  PCINT6
#define WHEEL_PCIE     PCIE0

/* leds */
#define LED_R_PORT PORTC
#define LED_R_DDR  DDRC
#define LED_R PC6
#define LED_G_PORT PORTC
#define LED_G_DDR DDRC
#define LED_G PC5
#define LED_B_PORT PORTB
#define LED_B_DDR  DDRB
#define LED_B PB7

#define led_r_off()    LED_R_PORT |=  _BV(LED_R)
#define led_r_on()     LED_R_PORT &= ~_BV(LED_R)
#define led_r_toggle() LED_R_PORT ^=  _BV(LED_R)
#define led_g_off()    LED_G_PORT |=  _BV(LED_G)
#define led_g_on()     LED_G_PORT &= ~_BV(LED_G)
#define led_g_toggle() LED_G_PORT ^=  _BV(LED_G)
#define led_b_off()    LED_B_PORT |=  _BV(LED_B)
#define led_b_on()     LED_B_PORT &= ~_BV(LED_B)
#define led_b_toggle() LED_B_PORT ^=  _BV(LED_B)

/* DF, on SPI */
#define DF_CS_PORT PORTD
#define DF_CS_DDR DDRD
#define DF0_CS PD0
#define DF1_CS PD1

/* PLN, on USART */
#define PLN_IT_PORT PORTD
#define PLN_IT_DDR  DDRD
#define PLN_IT_PIN  PIND
#define PLN_IT PD6
#define PLN_CS_PORT PORTB
#define PLN_CS_DDR  DDRB
#define PLN_CS PB4

#define pln_cs_h() (PLN_CS_PORT |=  _BV(PLN_CS))
#define pln_cs_l() (PLN_CS_PORT &= ~_BV(PLN_CS))

#define USART_DDR  DDRD
#define USART_MISO PD2
#define USART_MOSI PD3
#define USART_SCK  PD5

struct mouse_packet {
  uint8_t btns;
  int16_t x;
  int16_t y;
  int8_t z;
};

extern struct mouse_packet packet;

void prepare_packet (void);
