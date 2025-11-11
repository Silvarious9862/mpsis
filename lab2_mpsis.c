// Бобрик В.Ю. 250541 L2
// Подсистема прерываний 
#include <msp430.h>
// --- debounce settings ---
#define BTN_SAMPLES 10
#define BTN_THRESHOLD 8

// --- btn ---
#define BTN_S1_PORT_IN P1IN
#define BTN_S1_PORT_DIR P1DIR
#define BTN_S1_PORT_OUT P1OUT
#define BTN_S1_PORT_REN P1REN
#define BTN_S1_PORT_IE P1IE
#define BTN_S1_PORT_IES P1IES
#define BTN_S1_PORT_IFG P1IFG
#define BTN_S1_PORT_SEL P1SEL
#define BTN_S1_PORT_SEL2 P1SEL2
#define BTN_S1_BIT BIT7 // S1 = P1.7

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
#define BLINK_TICKS 20			 // led blinkings
volatile unsigned char mode = 0; // working mode
volatile unsigned char blink_phase = 0;
volatile unsigned int blink_cnt = 0;
volatile unsigned char btn_prev = 0;

void init_hw(void)
{
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT

	// led1
	LED1_PORT_DIR |= LED1_BIT; // out
	LED1_PORT_OUT |= LED1_BIT; // enable
	LED1_PORT_SEL &= ~LED1_BIT; // gpio
#ifdef P1SEL2
	LED1_PORT_SEL2 &= ~LED1_BIT;
#endif

	// led3
	LED3_PORT_DIR |= LED3_BIT; // out
	LED3_PORT_OUT |= LED3_BIT; // enable
	LED3_PORT_SEL &= ~LED3_BIT; // gpio
#ifdef P8SEL2
	LED3_PORT_SEL2 &= ~LED3_BIT;
#endif

	// btn1
	BTN_S1_PORT_DIR &= ~BTN_S1_BIT; // in
	BTN_S1_PORT_REN |= BTN_S1_BIT;	// ren
	BTN_S1_PORT_OUT |= BTN_S1_BIT;	// ren up
	BTN_S1_PORT_SEL &= ~BTN_S1_BIT; // GPIO
#ifdef P1SEL2
	BTN_S1_PORT_SEL2 &= ~BTN_S1_BIT;
#endif

	// timerA0
	TA0CCR0 = 1250 - 1; // from 0 to 1249
	//		SMCLK | divider 8 | count up | clear
	TA0CTL = TASSEL_2 | ID_3 | MC_1 | TACLR; 
	TA0CCTL0 = CCIE; // int by comparison enable
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
	__enable_interrupt();
	__low_power_mode_3();
	return 0;
}

// register int handler
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{
	// btn 
	unsigned char btn_now = debounce_S1();
	if (btn_prev == 1 && btn_now == 0)
	{
		mode = (mode + 1) & 3;
	}
	btn_prev = btn_now;

	// blink
	blink_cnt++;
	if (blink_cnt >= 25)
	{
		blink_cnt = 0;
		blink_phase ^= 1;
	}

	// mode
	switch (mode)
	{
	case 0: // led1 + led3
		LED1_PORT_OUT |= LED1_BIT;
		LED3_PORT_OUT |= LED3_BIT;
		break;
	case 1: // led3 blink
		LED1_PORT_OUT &= ~LED1_BIT;
		if (blink_phase)
			LED3_PORT_OUT |= LED3_BIT;
		else
			LED3_PORT_OUT &= ~LED3_BIT;
		break;
	case 2: // led1 blink
		LED3_PORT_OUT &= ~LED3_BIT;
		if (blink_phase)
			LED1_PORT_OUT |= LED1_BIT;
		else
			LED1_PORT_OUT &= ~LED1_BIT;
		break;
	case 3: // led 1 + led 3 
		LED1_PORT_OUT |= LED1_BIT;
		LED3_PORT_OUT |= LED3_BIT;
		mode = 0; // goto mode 0
		break;
	}
}
