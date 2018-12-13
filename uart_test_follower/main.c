/*
 * CS122A_Lab_02_pt1.c
 *
 * Created: 10/4/2018 2:31:44 PM
 * Author : Kushagra Singh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart_ATmega1284.h"
#include "bit.h"

volatile unsigned char TimerFlag = 0;
unsigned char prevAlpha = 0;
unsigned char alpha = 0;
unsigned char numSame = 0;
unsigned char count = 0;
unsigned char valueReceived = 0;
unsigned char prevValue = 0;
unsigned char buffer = 0;
unsigned char bufferVal = 0;

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

enum SM1_States { SM1_init, SM1_G, SM1_J, SM1_Q, SM1_H, SM1_P, SM1_M, SM1_N, SM1_I, SM1_W, SM1_A, SM1_Y, SM1_L, SM1_K, SM1_B, SM1_F, SM1_U, SM1_V, SM1_R, SM1_S, SM1_T, SM1_X, SM1_Z, SM1_C, SM1_D, SM1_E, SM1_O } SM1_State;

TickFct_Decoder() {
	switch(SM1_State) { // Transitions
		case -1:
		SM1_State = SM1_init;
		break;
		case SM1_init:
			if (valueReceived == 0b11001000) {
				SM1_State = SM1_G;
				count = 0;
			}
			else if (valueReceived == 0b11000001) {
				SM1_State = SM1_J;
				count = 0;
			}
			else if (valueReceived == 0b11011000) {
				SM1_State = SM1_Q;
				count = 0;
			}
			else if (valueReceived == 0b11001100) {
				SM1_State = SM1_H;
				count = 0;
			}
			else if (valueReceived == 0b11011111) {
				SM1_State = SM1_M;
				count = 0;
			}
			else if (valueReceived == 0b11100001) {
				SM1_State = SM1_I;
				count = 0;
			}
			else if (valueReceived == 0b11101110) {
				SM1_State = SM1_W;
				count = 0;
			}
			else if (valueReceived == 0b11110000) {
				SM1_State = SM1_A;
				count = 0;
			}
			else if (valueReceived == 0b11110001) {
				SM1_State = SM1_Y;
				count = 0;
			}
			else if (valueReceived == 0b11111000) {
				SM1_State = SM1_L;
				count = 0;
			}
			else if (valueReceived == 0b11111100) {
				SM1_State = SM1_K;
				count = 0;
			}
			else if (valueReceived == 0b11101111) {
				SM1_State = SM1_B;
				count = 0;
			}
			else if (valueReceived == 0b11101100) {
				SM1_State = SM1_U;
				count = 0;
			}
			else if (valueReceived == 0b11101000) {
				SM1_State = SM1_S;
				count = 0;
			}
			else if (valueReceived == 0b11111111) {
				SM1_State = SM1_C;
				count = 0;
			}
			break;
			case SM1_G:
			if (!(valueReceived == 0b11001000)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_J:
			if (!(valueReceived == 0b11000001)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_Q:
			if (!(valueReceived == 0b11011000)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_H:
			if (!(valueReceived == 0b11001100)) {
				SM1_State = SM1_init;
			}
			else if (valueReceived == 0b11001100 && count > 60) {
				SM1_State = SM1_P;
				count = 0;
			}
			break;
		case SM1_P:
			if (!(valueReceived == 0b11001100)) {
				SM1_State = SM1_H;
			}
			break;
		case SM1_M:
			if (!(valueReceived == 0b11011111)) {
				SM1_State = SM1_init;
			}
			else if (valueReceived == 0b11011111 && count > 60) {
				SM1_State = SM1_N;
				count = 0;
			}
			break;
		case SM1_N:
			if (!(valueReceived == 0b11011111)) {
				SM1_State = SM1_M;
			}
			break;
		case SM1_I:
			if (!(valueReceived == 0b11100001)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_W:
			if (!(valueReceived == 0b11101110)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_A:
			if (!(valueReceived == 0b11110000)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_Y:
			if (!(valueReceived == 0b11110001)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_L:
			if (!(valueReceived == 0b11111000)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_K:
			if (!(valueReceived == 0b11111100)) {
				SM1_State = SM1_init;
			}
			break;
		case SM1_B:
			if (!(valueReceived == 0b11101111)) {
				SM1_State = SM1_init;
			}
			else if (valueReceived == 0b11101111 && count > 60) {
				SM1_State = SM1_F;
				count = 0;
			}
			break;
		case SM1_F:
			if (!(valueReceived == 0b11101111)) {
				SM1_State = SM1_B;
			}
			break;
		case SM1_U:
			if (!(valueReceived == 0b11101100)) {
				SM1_State = SM1_init;
			}
			else if (valueReceived == 0b11101100 && count > 60) {
				SM1_State = SM1_V;
				count = 0;
			}
			break;
		case SM1_V:
			if (!(valueReceived == 0b11101100)) {
				SM1_State = SM1_U;
			}
			else if (valueReceived == 0b11101100 && count > 60) {
				SM1_State = SM1_R;
				count = 0;
			}
			break;
		case SM1_R:
			if (!(valueReceived == 0b11101100)) {
				SM1_State = SM1_V;
			}
			break;
		case SM1_S:
			if (!(valueReceived == 0b11101000)) {
				SM1_State = SM1_init;
			}
			else if (valueReceived == 0b11101000 && count > 60) {
				SM1_State = SM1_T;
				count = 0;
			}
			break;
		case SM1_T:
			if (!(valueReceived == 0b11101000)) {
				SM1_State = SM1_S;
			}
			else if (valueReceived == 0b11101000 && count > 60) {
				SM1_State = SM1_X;
				count = 0;
			}
			break;
		case SM1_X:
			if (!(valueReceived == 0b11101000)) {
				SM1_State = SM1_T;
			}
			else if (valueReceived == 0b11101000 && count > 60) {
				SM1_State = SM1_Z;
				count = 0;
			}
			break;
		case SM1_Z:
			if (!(valueReceived == 0b11101000)) {
				SM1_State = SM1_X;
			}
			break;
		case SM1_C:
			if (!(valueReceived == 0b11111111)) {
				SM1_State = SM1_init;
			}
			else if (valueReceived == 0b11111111 && count > 60) {
				SM1_State = SM1_D;
				count = 0;
			}
			break;
		case SM1_D:
			if (!(valueReceived == 0b11111111)) {
				SM1_State = SM1_C;
			}
			else if (valueReceived == 0b11111111 && count > 60) {
				SM1_State = SM1_E;
				count = 0;
			}
			break;
		case SM1_E:
			if (!(valueReceived == 0b11111111)) {
				SM1_State = SM1_D;
			}
			else if (valueReceived == 0b11111111 && count > 60) {
				SM1_State = SM1_O;
				count = 0;
			}
			break;
		case SM1_O:
			if (!(valueReceived == 0b11111111)) {
				SM1_State = SM1_E;
			}
			break;
		default:
		SM1_State = SM1_init;
	} // Transitions

	switch(SM1_State) { // State actions
		case SM1_init:
			alpha = 0xFF;
			count = 0;
			break;
		case SM1_G:
			//G
			alpha = 7;
			count = count + 1;
			break;
		case SM1_J:
			//alpha value J
			alpha = 10;
			count = count + 1;
			break;
		case SM1_Q:
			//alpha value Q
			alpha = 17;
			count = count + 1;
			break;
		case SM1_H:
			//alpha Value H
			alpha = 8;
			count = count + 1;
			break;
		case SM1_P:
			//alpha Value P
			alpha = 16;
			count = count + 1;
			break;
		case SM1_M:
			//alpha Value M
			alpha = 13;
			count = count + 1;
			break;
		case SM1_N:
			//alpha Value N
			alpha = 14;
			count = count + 1;
			break;
		case SM1_I:
			//alpha Value I
			alpha = 9;
			count = count + 1;
			break;
		case SM1_W:
			//alpha Value W
			alpha = 23;
			count = count + 1;
			break;
		case SM1_A:
			//alpha Value A
			alpha = 1;
			count = count + 1;
			break;
		case SM1_Y:
			//alpha Value Y
			alpha = 25;
			count = count + 1;
			break;
		case SM1_L:
			//alpha Value L
			alpha = 12;
			count = count + 1;
			break;
		case SM1_K:
			//alpha Value K
			alpha = 11;
			count = count + 1;
			break;
		case SM1_B:
			//alpha Value B
			alpha = 2;
			count = count + 1;
			break;
		case SM1_F:
			//alpha Value F
			alpha = 6;
			count = count + 1;
			break;
		case SM1_U:
			//alpha Value U
			alpha = 21;
			count = count + 1;
			break;
		case SM1_V:
			//alpha Value V
			alpha = 23;
			count = count + 1;
			break;
		case SM1_R:
			//alpha Value R
			alpha = 18;
			count = count + 1;
			break;
		case SM1_S:
			//alpha Value S
			alpha = 19;
			count = count + 1;
			break;
		case SM1_T:
			//alpha Value T
			alpha = 20;
			count = count + 1;
			break;
		case SM1_X:
			//alpha Value X
			alpha = 24;
			count = count + 1;
			break;
		case SM1_Z:
			//alpha Value Z
			alpha = 26;
			count = count + 1;
			break;
		case SM1_C:
			//alpha Value C
			alpha = 3;
			count = count + 1;
			break;
		case SM1_D:
			//alpha Value D
			alpha = 4;
			count = count + 1;
			break;
		case SM1_E:
			//alpha Value E
			alpha = 5;
			count = count + 1;
			break;
		case SM1_O:
			//alpha Value O
			alpha = 15;
			count = count + 1;
			break;
		default: // ADD default behaviour below
		break;
	} // State actions

}

enum SM5_States { SM5_s1} SM5_State;

void TickReceive(){
	switch(SM5_State) { // Transitions
		case -1:
		SM5_State = SM5_s1;
		break;
		case SM5_s1:
		if (1) {
			SM5_State = SM5_s1;
		}
		break;
		default:
		SM5_State = SM5_s1;
	} // Transitions

	switch(SM5_State) { // State actions
		case SM5_s1:
			if(USART_HasReceived(0)){
				/*USART_(0x00,0);*/
				valueReceived = (USART_Receive(0));
				valueReceived = ~valueReceived;
			}
			USART_Flush(0);
		break;
		default: // ADD default behaviour below
		break;
	} // State actions
}

enum SM2_States { SM2_s1} SM2_State;

void EncodeBuffer(char value, char alpha){
	if(buffer < 60){
		if(bufferVal != value){
			buffer = buffer + 1;
		}else{
			buffer = 0;
			prevAlpha = alpha;
		}
	}else{
		bufferVal = value;
		prevAlpha = alpha;
		buffer = 0;
	}
}

void DecodeAlpha(){
	switch(SM2_State) { // Transitions
		case -1:
		SM2_State = SM2_s1;
		break;
		case SM2_s1:
		if (1) {
			SM1_State = SM2_s1;
		}
		break;
		default:
		SM2_State = SM2_s1;
	} // Transitions

	switch(SM2_State) { // State actions
		case SM2_s1:
			if(prevValue == valueReceived){
				numSame = numSame + 1;
			}else{
				numSame = 0;
			}
			prevValue = valueReceived;
			
			if(valueReceived == 0b11011001 || valueReceived == 0b11111001){
				alpha = 0xFF; //denotes Space
				if (numSame > 60){
					//alpha value ' '
					alpha = 0;
				}
			}else if(~(valueReceived & 0b00100000)){
				if(valueReceived == 0b11001000){	
					//alpha value G
					alpha = 7;
					EncodeBuffer(valueReceived,alpha);
				}else if (valueReceived == 0b11000001){
					//alpha value J
					alpha = 10;
					EncodeBuffer(valueReceived,alpha);
				}else if (valueReceived == 0b11011000){
					//alpha value Q
					alpha = 17;
					EncodeBuffer(valueReceived,alpha);
				}else if (valueReceived = 0b11001100){
					//either H or P
					if(numSame < 100){
						//alpha Value H
						alpha = 8;
					}else{
						//alpha Value P
						alpha = 16;
					}
					EncodeBuffer(valueReceived,alpha);
				}else{
					//either M or N
					if(numSame < 100){
						//alpha Value M
						alpha = 13;
					}else{
						//alpha Value N
						alpha = 14;
					}
					EncodeBuffer(valueReceived,alpha); 
				}
			} else {
				if(valueReceived == 0b11100001){
					//alpha Value I
					alpha = 9;
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11101110){
					//alpha Value W
					alpha = 23;
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11110000){
					//alpha Value A
					alpha = 1;
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11110001){
					//alpha Value Y
					alpha = 25;
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11111000){
					//alpha Value L
					alpha = 12;
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11111100){
					//alpha Value K
					alpha = 11;
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11101111){
					//B or F
					if(numSame < 100){
						//alpha Value B
						alpha = 2;
					}else{
						//alpha Value F
						alpha = 6;
					}
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11101100){
					//Either U,V, or R
					if(numSame < 100){
						//alpha Value R
						alpha = 18;
					}else if (numSame < 160){
						//alpha Value U
						alpha = 21;
					}else{
						//alpha Value V
						alpha = 23;
					}
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11101000){
					//alpha Value S,T,X,Z
					if(numSame < 100){
						//alpha Value S
						alpha = 19;
					}else if (numSame < 160){
						//alpha Value T
						alpha = 20;
					}else if (numSame < 220){
						//alpha Value X
						alpha = 24;
					}else{
						//alpha Value Z
						alpha = 26;
					}
					EncodeBuffer(valueReceived,alpha);
				}else if(valueReceived == 0b11111111){
					//Either C,D,E,O
					if(numSame < 100){
						//alpha Value C
						alpha = 3;
					}else if (numSame < 160){
						//alpha Value D
						alpha = 4;
					}else if (numSame < 220){
						//alpha Value E
						alpha = 5;
					}else{
						//alpha Value O
						alpha = 15;
					}
					EncodeBuffer(valueReceived,alpha);
				}else{
					alpha = 0xFF;
				}
			}
		break;
		default: // ADD default behaviour below
		break;
	} // State actions
}

enum SM3_States { SM3_s1} SM3_State;

void PrintChar(){
	switch(SM3_State) { // Transitions
		case -1:
		SM3_State = SM3_s1;
		break;
		case SM3_s1:
		if (1) {
			SM1_State = SM3_s1;
		}
		break;
		default:
		SM3_State = SM3_s1;
	} // Transitions

	switch(SM3_State) { // State actions
		case SM3_s1:
			
		break;
		default: // ADD default behaviour below
		break;
	} // State actions
}

int main(void)
{
    /* Replace with your application code */
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PINC = 0xFF; // Switches
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	
	initUSART(0);
	//DecodeAlpha(0);
	//PrintChar(0);
	TimerSet(50);
	TimerOn();
	
	SM1_State = -1;
	SM5_State = -1;
	SM2_State = -1;
	prevAlpha = 0xAA;
	PORTB = 0x00;
	prevValue = 0xAA;
	valueReceived = 0xAA;
	//PORTB = 0x01;
	//LCD_init();
    while (1) 
    {
		TickReceive();
		if(~PINC & 0x80){
			//Debugging Mode
			unsigned char temp = ~PINC;
			temp = temp & 0b01111111;
			if(temp == 0x00){
				PORTB = valueReceived;
			}else{
				valueReceived = ~PINC;
				valueReceived = valueReceived | 0b11000000;
				PORTB = valueReceived;
			}
		}else{
			unsigned char temp = ~PINC;
			temp = temp & 0b011111111;
			if(!(temp == 0x00)){
				valueReceived = ~PINC;
				valueReceived = valueReceived | 0b11000000;
			}
			TickFct_Decoder();
			PORTB = alpha;
		}
		
		TickFct_Decoder();
		while (!TimerFlag);
		TimerFlag = 0;
    }
}

