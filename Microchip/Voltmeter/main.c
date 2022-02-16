/*
 * Voltmeter.c
 *
 * Created: 16.02.2022 21:09:13
 * Author : vadim
 */ 

# define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SEG7_DDR DDRD
#define SEG7_PORT PORTD

#define VREF 5
#define COEF_DIV 4

int r1_1000 = 0;
int r2_100 = 0;
int r3_10 = 0;
int r4_1 = 0;

int digits [10] =
{
	0b00111111, //0
	0b00000110, //1
	0b01011011, //2
	0b01001111, //3
	0b01100110, //4
	0b01101101, //5
	0b01111101, //6
	0b00000111, //7
	0b01111111, //8
	0b01101111, //9
	0b10000000  //.
};

void chislo (unsigned int vse_chislo)
{
	r1_1000 = vse_chislo/1000;
	r2_100 = vse_chislo%1000/100;
	r3_10 = vse_chislo%100/10;
	r4_1 = vse_chislo%10;
}

int z = 0;

float voltage = 0;

ISR (TIMER0_OVF_vect) // T = (8*256)/1000000 ~= 2ms --> interrupt
{
	z++;
	if (z > 4) z = 1;
	
	if (z == 1)
	{
		PORTB |= (1<<1) | (1<<2) | (1<<3);	// switch off 2, 3 and 4 bits
		PORTB &= ~(1<<0);			// switch on 1 bit
		SEG7_PORT = digits [r1_1000];
		
	}
	else if(z == 2)
	{
		PORTB |= (1<<0) | (1<<2) | (1<<3);	// switch off 1, 3 and 4 bits
		PORTB &= ~(1<<1);			// switch on 2 bit
		SEG7_PORT = digits [r2_100];
		
		PORTD |= (1<<7);//.
	}
	else if(z == 3)
	{
		PORTB |= (1<<0) | (1<<1) | (1<<3);	// switch off 1, 2 and 4 bits
		PORTB &= ~(1<<2);			// switch on 3 bit
		SEG7_PORT = digits [r3_10];
	}
	else if(z == 4)
	{
		PORTB |= (1<<0) | (1<<1) | (1<<2);	// switch off 1, 2 and 3 bits
		PORTB &= ~(1<<3);			// switch on 4 bit
		SEG7_PORT = digits [r4_1];
	}
}


ISR (ADC_vect)
{
	
	voltage = (float)ADC*VREF*100/1023*COEF_DIV;
	
	chislo(voltage);
}

void ADC_settings (void)
{
	DDRB |= (1<<1) | (1<<0);
	PORTB &= ~((1<<0) | (1<<0));
	
	DDRC &= (1<<3);
	
	ADCSRA |= (1<<7); //ADC enable
	ADCSRA |= (1<<5); //ADC free running select
	
	//Fd = Fclk/8
	ADCSRA |= (1<<1) | (1<<0);
	ADCSRA &= ~(1<<2);
	
	//External 5V
	ADMUX &= ~(1<<7);
	ADMUX |= (1<<6);
	
	ADMUX &= ~(1<<5); //right
	
	//3 channel
	ADMUX &= ~((1<<3) | (1<<2));
	ADMUX |= (1<<1) | (1<<0);
	
	ADCSRA |= (1<<3); // Enable ADC interrupt
	
	ADCSRA |= (1<<6); //ADC start
}


int main(void)
{
	SEG7_DDR = 0xff;
	SEG7_PORT = 0x00;
	
	DDRB |= (1<<0) | (1<<1) | (1<<2) | (1<<3);
	PORTB |= (1<<0) | (1<<1) | (1<<2) | (1<<3);
	
	TCCR0 |= (1<<1); // /8
	TCNT0 = 0;
	TIMSK |= (1<<0); //Timer/Counter0 Overflow Interrupt Enable
	
	ADC_settings ();
	
	sei();
	
	
	
	while (1)
	{
	}
}

