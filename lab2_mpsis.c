// Бобрик В.Ю. 250541 L2
// Подсистема прерываний
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
#define BTN_S1_PORT_IE P1IE
#define BTN_S1_PORT_IES P1IES
#define BTN_S1_PORT_IFG P1IFG
#define BTN_S1_BIT BIT7 // кнопка S1 = P1.7

// --- led ---
#define LED1_PORT_OUT P1OUT
#define LED1_PORT_DIR P1DIR
#define LED1_PORT_SEL P1SEL
#define LED1_PORT_SEL2 P1SEL2
#define LED1_BIT BIT0 // LED1 = P1.0

#define LED3_PORT_OUT P8OUT
#define LED3_PORT_DIR P8DIR
#define LED3_PORT_SEL P8SEL
#define LED3_PORT_SEL2 P8SEL2
#define LED3_BIT BIT2 // LED3 = P8.2

// ----
#define BLINK_COUNT 15
volatile unsigned char mode = 0; // режим работы
volatile unsigned char blink_phase = 0;
volatile unsigned int blink_cnt = 0;

void init_hw(void) // стартовая настройка
{
	WDTCTL = WDTPW + WDTHOLD; // остановить WDT

	// led1
	LED1_PORT_DIR |= LED1_BIT; // режим выход
	LED1_PORT_OUT |= LED1_BIT; // включить диод
	LED1_PORT_SEL &= ~LED1_BIT; // gpio

	// led3
	LED3_PORT_DIR |= LED3_BIT; // режим выход
	LED3_PORT_OUT |= LED3_BIT; // включить диод
	LED3_PORT_SEL &= ~LED3_BIT; // gpio

	// кнопка 1
	BTN_S1_PORT_DIR &= ~BTN_S1_BIT; // режим вход
	BTN_S1_PORT_REN |= BTN_S1_BIT;	// резистор
	BTN_S1_PORT_OUT |= BTN_S1_BIT;	// подтяжка резистора вверх
	BTN_S1_PORT_SEL &= ~BTN_S1_BIT; // gpio

	// прерывание по кнопке
    BTN_S1_PORT_IE |= BTN_S1_BIT;   // разрешить прерывание
    BTN_S1_PORT_IES |= BTN_S1_BIT;  // сначала ловим спад (нажатие)
    BTN_S1_PORT_IFG &= ~BTN_S1_BIT; // сброс флага

	// таймер A0
	TA0CCR0 = 1250 - 1; // от 0 до 1249 (ровно 10 мс)
	//		SMCLK | делитель 8 | счет вверх | очистить
	TA0CTL = TASSEL_2 | ID_3 | MC_1 | TACLR;
	TA0CCTL0 = CCIE; // разрешить прерывание по равенству
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
	init_hw(); 				// стартовая настройка
	__enable_interrupt();	// разрешить прерывания
	__low_power_mode_3();	// режим низкого потребления
	return 0;
}

// регистрация обработчика прерывания по таймеру А0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{
	// мигание
	blink_cnt++;
	if (blink_cnt >= BLINK_COUNT)
	{
		blink_cnt = 0;
		blink_phase ^= 1;
	}

	// режим работы
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
		mode = 0; // переход в режим 0
		break;
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if (BTN_S1_PORT_IFG & BTN_S1_BIT) // если есть прерывание от кнопки S1
    {
        if (BTN_S1_PORT_IES & BTN_S1_BIT) // если это задний фронт
        {
            if (debounce_S1())
            {
                // переключаем на передний фронт (отпускание)
                BTN_S1_PORT_IES &= ~BTN_S1_BIT;
            }
        }
        else // если это передний фронт
        {
            if (!debounce_S1())
            {
                mode = (mode + 1) & 3; // смена режима
                // переключаем на задний фронт (нажатие)
                BTN_S1_PORT_IES |= BTN_S1_BIT;
            }
        }
        BTN_S1_PORT_IFG &= ~BTN_S1_BIT; // сброс флага
    }
}