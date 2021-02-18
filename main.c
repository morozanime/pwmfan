#define	F_CPU	16000000UL
#define	BAUD	((F_CPU/8UL/115200UL)-1UL)

#include	<avr/interrupt.h>
#include	<inttypes.h>
#include	<util/delay.h>
#include	<string.h>
#include	<stdio.h>

uint8_t pwm[6];

uint8_t cnt[6];
uint8_t rpm[6];

char tx_buff[256];
char rx_buff[256];
volatile uint8_t rx_pos_put;
uint8_t rx_pos_get;

void uart_put(char *str) {
	UCSR0B &= ~0x20;
	strncpy(tx_buff, str, sizeof(tx_buff));
	UCSR0B |= 0x20;
}

void rpm_calc(void) {
	static uint16_t t0 = 0;
	static uint8_t cnt_last[6] = { 0 };
	uint16_t t1 = TCNT1;
	uint16_t dt = t1 - t0;
	if (dt >= (uint16_t) (F_CPU / 1024UL / 2)) {
		t0 = t1;
		for (uint8_t i = 0; i < 6; i++) {
			uint8_t c = cnt[i];
			rpm[i] = c - cnt_last[i];
			cnt_last[i] = c;
		}
	}
}

uint8_t recv_pkt(void) {
	if (rx_pos_put != rx_pos_get) {
		uint8_t i = rx_pos_get;
		uint8_t state = 0, pi = 0, pw = 0;
		for (; i != rx_pos_put; i++) {
			uint8_t c = rx_buff[i];
			if (state == 0) {
				if (c >= '0' && c <= '5') {
					pi = c - '0';
					state = 1;
				}
			} else if (state == 1) {
				if (c == ':') {
					state = 2;
					pw = 0;
				}
			} else if (state == 2) {
				if (c >= '0' && c <= '5') {
					pw *= 10;
					pw += c - '0';
				} else if (c == ',') {
					pwm[pi] = pw;
					state = 0;
				}
			}
		}
		rx_pos_get = i;
		return 1;
	} else
		return 0;
}

void send_state(void) {
	uint8_t ii = 0, j;
	char buff[50];
	for (uint8_t i = 0; i < 6; i++) {
		j = sprintf(buff + ii, "%d,", (uint16_t) rpm[i] * 30);
		ii += j;
	}
	sprintf(buff + ii, "\n");
	uart_put(buff);
}

//-------------------------------------------------------------------
int main(void) {
	DDRB = 0x3F;
	PORTB = 0x00;

	DDRD = 0xFE;
	PORTD = 0xFD;

	DDRC = 0x00;
	PORTC = 0x3F;

	memset(pwm, 0, sizeof(pwm));

	TCCR0A = 0x00;
	TCCR0B = 0x01;
	TIMSK0 = 0x01;

	/*115200 8n1*/
	UBRR0H = (uint8_t) (BAUD >> 8);
	UBRR0L = (uint8_t) BAUD;
	UCSR0A = 0x02;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = 0x06;

	TCCR1B = 0x05;

	rx_pos_put = 0;
	sei();
	while (1) {
		rpm_calc();
		if (recv_pkt()) {
			send_state();
		}
	}
}

ISR(TIMER0_OVF_vect) {
	static uint8_t counter = 0;
	counter++;
	uint8_t mask = 0;
	for (uint8_t i = 0; i < 6; i++) {
		mask >>= 1;
		if (counter < pwm[i])
			mask |= 0x20;
	}
	PORTB = mask;

	static uint8_t pin_last = 0;
	uint8_t pins = PINC;
	uint8_t pins_changed = pin_last ^ pins;
	pin_last = pins;
	for (uint8_t i = 0; i < 6; i++) {
		if (pins_changed & 1)
			cnt[i]++;
		pins_changed >>= 1;
	}
}

ISR(USART_UDRE_vect) {
	static uint8_t tx_pos = 0;
	UDR0 = tx_buff[tx_pos++];
	if (tx_buff[tx_pos] == 0) {
		UCSR0B &= ~0x20;
		tx_pos = 0;
	}
}

ISR(USART_RX_vect) {
	static uint8_t pos = 0;
	uint8_t c = UDR0;
	if (c >= 0x20 && c < 0x80) {
		rx_buff[pos++] = c;
	} else if (c == '\n' || c == '\r') {
		rx_pos_put = pos;
	}
}
