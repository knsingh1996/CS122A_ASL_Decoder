/*
 * GccApplication3.c
 *
 * Created: 10/30/2018 3:17:18 PM
 * Author : Kushagra Singh
 */ 

#include <avr/io.h>

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}


int main(void)
{
    DDRB = 0xFF; PORTB = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    ADC_init();
    // Sets PA0 to be input pin for A2D conversion
    Set_A2D_Pin(0x01);

    while (1)
    {
	    unsigned short my_short = ADC; //0xABCD
	    unsigned char my_char = (char)my_short; // my_char = 0xCD
	    PORTB = my_char;
	    my_char = (char)(my_short >> 8); // my_char = 0xAB
	    PORTD = my_char;
    }
}

