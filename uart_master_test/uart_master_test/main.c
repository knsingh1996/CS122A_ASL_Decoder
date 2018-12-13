/*
 * CS122A_Lab_02_pt1.c
 *
 * Created: 10/4/2018 2:31:44 PM
 * Author : Kushagra Singh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart_ATmega1284.h"

//Calibration Variables:
char count = 0x00;
int same = 0x00;
char alpha = 0x00;
char val = 0x00;
char calibrationDone = 0x00;
unsigned short orientationArr[3][3];
unsigned short fingerArr[3][3][5];
char orientation = 0x00;

volatile unsigned char TimerFlag = 0;

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	//OCR1A = 37.5;
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

// Valid values range between 0 and 7, where the value
void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}

//enum SM1_States { SM1_s1, SM1_s2 } SM1_State;
enum SM1_States { SM1_init, SM1_s2, SM1_s3, SM1_s4, SM1_s5, SM1_s6, SM1_s7, SM1_s8, SM1_s9, SM1_s10, SM1_s11, SM1_s12, SM1_s13, SM1_s14 } SM1_State;
// void TickFct(){
// 	switch(SM1_State) { // Transitions
// 		case -1:
// 		SM1_State = SM1_s1;
// 		break;
// 		case SM1_s1:
// 		if (1) {
// 			SM1_State = SM1_s2;
// 		}
// 		break;
// 		case SM1_s2:
// 		if (1) {
// 			SM1_State = SM1_s1;
// 		}
// 		break;
// 		default:
// 		SM1_State = SM1_s1;
// 	} // Transitions
// 
// 	switch(SM1_State) { // State actions
// 		case SM1_s1:
// 			//Toggle LED off
// 			PORTB = 0x00;
// 			//Send off signal to follower
// 			if(USART_IsSendReady(0)){
// 				unsigned short my_short = ADC; //0xABCD
// 				unsigned char my_char = (char)my_short; //0xCD
// 				//my_char = (char)(my_short >> 8); //AB
// 				//if(~PINB & 0x01){
// 				//	USART_Send(0xFF,0);
// 				//}else{
// 				//	USART_Send(0x00,0);
// 				//}
// 				USART_Send(my_char,0);
// 			}
// 			USART_Flush(0);
// 		break;
// 		case SM1_s2:
// 			//Toggle LED on
// 			PORTB = 0x01;
// 			//Send on signal to follower
// 			if(USART_IsSendReady(0)){
// 				unsigned short my_short = ADC; //0xABCD
// 				unsigned char my_char = (char)my_short;
// 				//my_char = (char)(my_short >> 8); //AB
// 				//if(~PINB & 0x01){
// 				//	USART_Send(0xFF,0);
// 				//}else{
// 				//	USART_Send(0x00,0);
// 				//}
// 				USART_Send(my_char,0);
// 			}
// 			USART_Flush(0);
// 		break;
// 		default: // ADD default behaviour below
// 		break;
// 	} // State actions
// }

void Counter(char orientation, char fingState){
	if(count == 0x00){
		PORTC = 0b01000000;
	} else if (count == 0x01){
		PORTC = 0b00100000;
	}else if (count == 0x02){
		PORTC = 0b00010000;
	}else if (count == 0x03){
		PORTC = 0b01110000;
		//if Orientation is straight
		Set_A2D_Pin(7);
		orientationArr[orientation][2] = ADC;
		Set_A2D_Pin(6);
		orientationArr[orientation][1] = ADC;
		Set_A2D_Pin(5);
		orientationArr[orientation][0] = ADC;
		Set_A2D_Pin(4);
		fingerArr[orientation][fingState][4] = ADC;
		Set_A2D_Pin(3);
		fingerArr[orientation][fingState][3] = ADC; 
		Set_A2D_Pin(2);
		fingerArr[orientation][fingState][2] = ADC; 
		Set_A2D_Pin(1);
		fingerArr[orientation][fingState][1] = ADC; 
		Set_A2D_Pin(0);
		fingerArr[orientation][fingState][0] = ADC; 
		//GET INPUT FOR STRAIGHT OPEN
	}
	count = count + 0x01;
}

TickFct_Calibrate() {
	switch(SM1_State) { // Transitions
		case -1:
		SM1_State = SM1_init;
		break;
		case SM1_init:
		if (1) {
			SM1_State = SM1_s2;
		}
		break;
		case SM1_s2:
		if (1) {
			SM1_State = SM1_s6;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s3:
		if (1) {
			SM1_State = SM1_s9;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s4:
		if (1) {
			SM1_State = SM1_s12;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s5:
		break;
		case SM1_s6:
		if (count > 0x04) {
			SM1_State = SM1_s7;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s7:
		if (count > 0x04) {
			SM1_State = SM1_s8;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s8:
		if (count > 0x04) {
			SM1_State = SM1_s3;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s9:
		if (count > 0x04) {
			SM1_State = SM1_s10;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s10:
		if (count > 0x04) {
			SM1_State = SM1_s11;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s11:
		if (count > 0x04) {
			SM1_State = SM1_s4;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s12:
		if (count > 0x04) {
			SM1_State = SM1_s13;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s13:
		if (count > 0x04) {
			SM1_State = SM1_s14;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		case SM1_s14:
		if (count > 0x04) {
			SM1_State = SM1_s5;
			PORTC = 0x00;
			count = 0x00;
		}
		break;
		default:
		SM1_State = SM1_init;
	} // Transitions

	switch(SM1_State) { // State actions
		case SM1_init:
		break;
		case SM1_s2:
		count = 0x00;
		val = 0b01000000;
				//USART: 0x01
		break;
		case SM1_s3:
		count = 0x00;
		val = 0b10000000;
		//USART: 0x02
		break;
		case SM1_s4:
		count = 0x00;
		val = 0b11000000;
		//USART: 0x03
		break;
		case SM1_s5:
		count = 0x00;
		calibrationDone = 0x01;
		val = 0x00;
		//USART: 0x00
		break;
		case SM1_s6:
			Counter(0x00,0x00);
			//GET INPUT FOR STRAIGHT OPEN
		//USART: 0x01
		break;
		case SM1_s7:
			Counter(0x00,0x01);
			//GET INPUT FOR STRAIGHT HALF
		//USART: 0x01
		break;
		case SM1_s8:
			Counter(0x00,0x02);
			//GET INPUT FOR STRAIGHT CLOSE
		//USART: 0x01
		break;
		case SM1_s9:
			Counter(0x01,0x00);
			//GET INPUT DOWN OPEN
		//USART: 0x01
		break;
		case SM1_s10:
			Counter(0x01,0x01);
			//GET INPUT FOR DOWN HALF
		//USART: 0x01
		break;
		case SM1_s11:
			Counter(0x01,0x02);
			//GET INPUT FOR DOWN CLOSE
		//USART: 0x01
		break;
		case SM1_s12:
			Counter(0x02,0x00);
			//GET INPUT FOR HORIZ OPEN
		//USART: 0x01
		break;
		case SM1_s13:
			Counter(0x02,0x01);
			//GET INPUTHORIZ HALF
		//USART: 0x01
		break;
		case SM1_s14:
			Counter(0x02,0x02);
			//GET INPUT HORIZ CLOSE
		//USART: 0x01
		break;
		default: // ADD default behaviour below
		break;
	} // State actions

}

enum SM2_States { SM2_init, SM2_s2 } SM2_State;

TickFct_Orient() {
	switch(SM2_State) { // Transitions
		case -1:
		SM2_State = SM2_init;
		break;
		case SM2_init:
		if (1) {
			SM2_State = SM2_s2;
		}
		break;
		case SM2_s2:
		if (1) {
			SM2_State = SM2_s2;
		}
		break;
		default:
		SM2_State = SM2_init;
	} // Transitions

	switch(SM2_State) { // State actions
		case SM2_init:
		break;
		case SM2_s2:
			Set_A2D_Pin(7);
			unsigned short c2 = ADC;
			Set_A2D_Pin(6);
			unsigned short c1 = ADC;
			Set_A2D_Pin(5);
			unsigned short c0 = ADC;
			unsigned int straightDiff = (c2-orientationArr[0][2])*(c2-orientationArr[0][2]) + 
						(c1-orientationArr[0][1])*(c1-orientationArr[0][1]) +
						(c0-orientationArr[0][0])*(c0-orientationArr[0][0]);
			
			unsigned int downDiff = (c2-orientationArr[1][2])*(c2-orientationArr[1][2]) +
						(c1-orientationArr[1][1])*(c1-orientationArr[1][1]) +
						(c0-orientationArr[1][0])*(c0-orientationArr[1][0]) ;
						
			unsigned int horizontalDiff = (c2-orientationArr[2][2])*(c2-orientationArr[2][2]) +
						(c1-orientationArr[2][1])*(c1-orientationArr[2][1]) +
						(c0-orientationArr[2][0])*(c0-orientationArr[2][0]);
						
			if(straightDiff <= downDiff && straightDiff <= horizontalDiff){
				PORTC = 0b01000000;
				orientation = 0x01;
				val = val & 0b11011111;
			} else if(downDiff <= straightDiff && downDiff <= horizontalDiff){
				PORTC = 0b00010000;
				orientation = 0x02;
				val = val | 0b00100000;
			} else{
				PORTC = 0b00010000;
				val = val | 0b00100000;
				orientation = 0x03;
			}
		//See what orientation it is and output to USART
		break;
		default: // ADD default behaviour below
		break;
	} // State actions

}

enum SM3_States { SM3_init, SM3_s2 } SM3_State;

TickFct_ReadFinger() {
	switch(SM3_State) { // Transitions
		case -1:
		SM3_State = SM3_init;
		break;
		case SM3_init:
		if (1) {
			SM3_State = SM3_s2;
		}
		break;
		case SM3_s2:
		if (1) {
			SM3_State = SM3_s2;
		}
		break;
		default:
		SM3_State = SM3_init;
	} // Transitions

	switch(SM3_State) { // State actions
		case SM3_init:
		break;
		case SM3_s2:
			if(~PINB & 0x01){
				val = val | 0b00010000;
				}else{
				val = val & 0b11101111;
			}
			
			if(~PINB & 0x02){
				val = val | 0b00001000;
				}else{
				val = val & 0b11110111;
			}
			
			if(~PINB & 0x04){
				val = val | 0b00000100;
				}else{
				val = val & 0b11111011;
			}
			
			if(~PINB & 0x08){
				val = val | 0b00000010;
				}else{
				val = val & 0b11111101;
			}
			
			if(~PINA & 0x01){
				val = val | 0b00000001;
				}else{
				val = val & 0b11111110;
			}
		break;
		default: // ADD default behaviour below
		break;
	} // State actions

}

int main(void)
{
    /* Replace with your application code */
	DDRA = 0x00; PINA = 0xFF;
	DDRB = 0x00; PINB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	
	initUSART(0);
	
	TimerSet(1);
	TimerOn();
	
	SM1_State = -1;
	SM2_State = -1;
	SM3_State = -1;
	
	ADC_init();
	Set_A2D_Pin(0);
	// Sets PA0 to be input pin for A2D conversion
	//orientationArr[0x00][0x00] = 0xFFAA;
    while (1) 
    {
		//PORTB = 0x01;
		//TickFct();
		if(calibrationDone == 0x00){
			TickFct_Calibrate();
			TimerSet(1000);
		}else{
			TimerSet(100);
			//0x01 = Vertical, 0x02 = Horizontal/Down
			TickFct_Orient();
			TickFct_ReadFinger();
			
		}
		//short mychar = ADC;
		if(USART_IsSendReady(0)){
			USART_Send(val,0);
		}
		USART_Flush(0);
		while (!TimerFlag);
		TimerFlag = 0;
    }
}

