// Бобрик В.Ю. 250541 L1
// Цифровой ввод-вывод
#include <msp430.h>
// --- настройки антидребезга ---
#define BTN_SAMPLES 10 // всего опросов
#define BTN_THRESHOLD 8 // 8 из 10 должны показать 0 на кнопке

// --- регистры кнопки ---
#define BTN_S1_PORT_IN P1IN
#define BTN_S1_PORT_DIR P1DIR
#define BTN_S1_PORT_OUT P1OUT
#define BTN_S1_PORT_REN P1REN
#define BTN_S1_PORT_SEL P1SEL
#define BTN_S1_BIT BIT7 // кнопка S1 = P1.7

// --- led ---
#define LED1_PORT_OUT P1OUT
#define LED1_PORT_DIR P1DIR
#define LED1_PORT_SEL P1SEL
#define LED1_PORT_SEL2 P1SEL2
#define LED1_BIT BIT0 // LED1 = P1.0

#define LED2_PORT_OUT P8OUT
#define LED2_PORT_DIR P8DIR
#define LED2_PORT_SEL P8SEL
#define LED2_PORT_SEL2 P8SEL2
#define LED2_BIT BIT1 // LED2 = P8.1

#define LED3_PORT_OUT P8OUT
#define LED3_PORT_DIR P8DIR
#define LED3_PORT_SEL P8SEL
#define LED3_PORT_SEL2 P8SEL2
#define LED3_BIT BIT2 // LED3 = P8.2

// ----
#define BLINK_TICKS 20	// длительность мигания
unsigned char mode = 0; // рабочий режим

void init_hw(void)
{
	WDTCTL = WDTPW + WDTHOLD; // остановить WDT

	// led1
	LED1_PORT_DIR |= LED1_BIT; // режим выход
	LED1_PORT_OUT |= LED1_BIT; // включить режим выхода
	LED1_PORT_SEL &= ~LED1_BIT; // gpio

	// led3
	LED3_PORT_DIR |= LED3_BIT; // режим выход
	LED3_PORT_OUT |= LED3_BIT; // включить режим выхода
	LED3_PORT_SEL &= ~LED3_BIT; // gpio

	// кнопка 1
	BTN_S1_PORT_DIR &= ~BTN_S1_BIT; // режим вход
	BTN_S1_PORT_REN |= BTN_S1_BIT;	// резистор
	BTN_S1_PORT_OUT |= BTN_S1_BIT;	// подтяжка резистора вверх
	BTN_S1_PORT_SEL &= ~BTN_S1_BIT; // gpio
}

unsigned char debounce_S1(void)
{
	unsigned int i, j, pressed_count = 0;
	for (i = 0; i < BTN_SAMPLES; i++)
	{
		if ((BTN_S1_PORT_IN & BTN_S1_BIT) == 0)
		{
			pressed_count++;
		}
		for (j = 0; j < 10; j++)
		{
			;
		}
	}
	return pressed_count >= BTN_THRESHOLD;
}

int main(void)
{
	init_hw();

	unsigned char btn_prev = 0, blink_phase = 0;
	unsigned int tick = 0;
	while (1)
	{

		// антидребезг
		unsigned char btn_now = debounce_S1();
		if (btn_prev == 1 && btn_now == 0)
		{
			mode = (mode + 1) & 3; // 0-1-2-0
		}
		btn_prev = btn_now;

		// мигания
		tick++;
		if (tick >= BLINK_TICKS)
		{
			tick = 0;
			blink_phase ^= 1; // xor led
		}

		// режимы
		switch (mode)
		{
		case 0: // led1 + led3
			LED1_PORT_OUT |= LED1_BIT;
			LED3_PORT_OUT |= LED3_BIT;
			break;
		case 1: // led3 мигает
			LED1_PORT_OUT &= ~LED1_BIT;
			if (blink_phase)
				LED3_PORT_OUT |= LED3_BIT;
			else
				LED3_PORT_OUT &= ~LED3_BIT;
			break;
		case 2: // led1 мигает
			LED3_PORT_OUT &= ~LED3_BIT;
			if (blink_phase)
				LED1_PORT_OUT |= LED1_BIT;
			else
				LED1_PORT_OUT &= ~LED1_BIT;
			break;
		case 3: // led 1 + led 3
			LED1_PORT_OUT |= LED1_BIT;
			LED3_PORT_OUT |= LED3_BIT;
			mode = 0; // идем в режим 0
			break;
		}
		// задержка 
		volatile unsigned int d;
		for (d = 0; d < 1000; d++)
		{
			;
		}
	}
}
