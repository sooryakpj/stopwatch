/*
 * stopwatch_youtube.c
 *
 * Created: 7/27/2016 10:05:10 PM
 *  Author: munta
 */ 



#include <stdio.h>
#include <msp430.h>
#include <intrinsics.h>
#define delay_ms(x)		__delay_cycles((long) x* 1000)
#define delay_us(x)		__delay_cycles((long) x)
#define RS 0x04
#define WR 0x05
#define EN 0x06

#define	LOWNIB(x)	P2OUT = (P2OUT & 0xF0) + (x & 0x0F)
#define CLEAR	0x01
#define DATA	0x0F






void button_conditions(void);
void display(void);
void convert(void);

unsigned int mil_sec = 0;
unsigned short sec = 0, min = 0, hour = 0, start_stop_flag = 0;
char str_mil_sec[4] = {'\0'}, str_sec[3] = {'\0'}, str_min[3] = {'\0'}, str_hour[3] = {'\0'};

void main(void)
{
        WDTCTL = WDTPW | WDTHOLD;
	P4DIR|=~((BIT2)|(BIT3));
        TACTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
        TAR=0x00;
        TACCR0=30;
	TACCTL0 = OUTMOD_5 | CCIE | ~CAP; 
       
	
	
	__enable_interrupt();
	void lcdInit();
	delay_ms(50);
	
	void lcdSetText(void);
	void lcdSetText(void);
	//initialization 
	
    while(1)
    {
		button_conditions();
		convert();
		if(start_stop_flag)
			display();
        //call functions
    }
}

#pragma vector=TIMERA1_VECTOR
__interrupt void Timer_A(void)
{
	
	if(start_stop_flag)
		mil_sec++;
	if(mil_sec >= 1000){
		mil_sec = 0;
		sec++;
	}
	if(sec >= 60){
		sec = 0;
		min++;
	}
	if(min >= 60){
		
		min = 0;
		hour++;
	}

}

void convert(void){
	str_mil_sec[3] = '\0';
	str_mil_sec[2] = (mil_sec % 10) + '0';				//435, 5
	str_mil_sec[1] = (mil_sec / 10) % 10 + '0';			//435, 43, 3
	str_mil_sec[0] = (mil_sec / 100) + '0';
        
	str_sec[2] = '\0';
	str_sec[1] = (sec % 10) + '0';
	str_sec[0] = (sec / 10) + '0';
	
        str_min[2] = '\0';
	str_min[1] = (min % 10) + '0';
	str_min[0] = (min / 10) + '0';
	
        str_hour[2] = '\0';
	str_hour[1] = (hour % 10) + '0';
	str_hour[0] = (hour / 10) + '0';
	
	return;
}

void display(void){
	
	delay_us(100);
	lcdSetText(str_mil_sec,13,1);
	delay_us(100);
	lcdSetText(str_sec,10,1);
	delay_us(100);
	lcdSetText(str_min,7,1);
	delay_us(100);
	lcdSetText(str_hour,4,1);
	delay_us(100);
	
	return;
}

void button_conditions(){
	
	if(!(P4IN & 0x02)){						//is start/stop pressed?
		
		if(start_stop_flag == 1){
			start_stop_flag = 0;
			TACTL = MC_0 | TACLR;
		}else{
			start_stop_flag = 1;
			 TACTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
		}
		while(!(P4IN & 0x02));
	}
	
	if(!(P4IN & 0x03)){
		
		start_stop_flag = 0;
		TACTL = MC_0 | TACLR;
		mil_sec = 0;
		sec = 0;
		min = 0;
		hour = 0;
		TAR = 0;
		display();
	}
	
	
	return;
}





void lcdInit() {
	delay_ms(100);
	// Wait for 100ms after power is applied.

	P4DIR = EN + RS + WR;  // Make pins outputs
	P4OUT = 0x03;  // Start LCD (send 0x03)
        P3DIR = DATA;
        
	void lcdTriggerEN(); // Send 0x03 3 times at 5ms then 100 us
	delay_ms(5);
	void lcdTriggerEN();
	delay_ms(5);
	void lcdTriggerEN();
	delay_ms(5);

	P4OUT = 0x02; // Switch to 4-bit mode
	lcdTriggerEN();
	delay_ms(5);

	lcdWriteCmd({0x38}); // 4-bit, 2 line, 5x8
        lcdWriteCmd(0x08); // Instruction Flow
	lcdWriteCmd(0x01); // Clear LCD
	lcdWriteCmd(0x06); // Auto-Increment
	lcdWriteCmd(0x0C); // Display On, No blink
}

void lcdTriggerEN() {
	P4OUT |= EN;
	P4OUT &= ~EN;
        return;
}

void lcdWriteData(unsigned char data) {
	P4OUT |= RS; // Set RS to Data
	LOWNIB(data >> 4); // Upper nibble
	lcdTriggerEN();
	LOWNIB(data); // Lower nibble
	lcdTriggerEN();
	delay_us(50); // Delay > 47 us
        return;
}

void lcdWriteCmd(unsigned char cmd) {
	P4OUT &= ~RS; // Set RS to Data
	LOWNIB(cmd >> 4); // Upper nibble
	lcdTriggerEN();
	LOWNIB(cmd); // Lower nibble
	lcdTriggerEN();
	delay_ms(5); // Delay > 1.5ms
        return;
}

void lcdSetText(const char* text, int x, int y) {
	int i;
	if (x < 16) {
		x |= 0x80; // Set LCD for first line write
		switch (y){
		case 1:
			x |= 0x40; // Set LCD for second line write
			break;
		case 2:
			x |= 0x60; // Set LCD for first line write reverse
			break;
		case 3:
			x |= 0x20; // Set LCD for second line write reverse
			break;
		}
		lcdWriteCmd(x);
	}
	i = 0;

	while (text[i] != '\0') {
		lcdWriteData(text[i]);
		i++;
	}
        return;
}

void lcdSetInt(int val, int x, int y){
	char number_string[16];
	sprintf(number_string, "%d", val); // Convert the integer to character string
	lcdSetText(number_string, x, y);
        return;
}

void lcdClear() {
	lcdWriteCmd(CLEAR);
        return;
}
