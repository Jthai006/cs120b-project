#include <avr/io.h>
#define F_CPU 1000000
#include <util/delay.h>
#include <stdlib.h>

#include "Nokia5110.c "
#include "timer.c "
#include "C:\Users\Jon\Desktop\includes\io.h"
#include "C:\Users\Jon\Desktop\includes\io.c"

int16_t x;
int16_t x1;
int16_t x2;
int16_t x3;
//#include "C:\Users\Jon\Desktop\includes\lcd.h"

typedef struct ball{
	
	char x1;
	char y1;
	char x2;
	char y2;
	char x3;
	char y3;
	char x4;
	char y4;
	char x5;
	char y5;
	char x6;
	char y6;
	char x7;
	char y7;
	char x8;
	char y8;
	char x9;
	char y9;
	
}ball;

ball bal[2];

typedef struct bar{
	
	char x1;
	char y1;
	char x2;
	char y2;
	char x3;
	char y3;
	char x4;
	char y4;
	char x5;
	char y5;
	char x6;
	char y6;
	char x7;
	char y7;
	
}bar;

bar num[2];

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

uint16_t ReadADC(uint8_t ch)
{
	//Select ADC Channel ch must be 0-7
	//ch=ch&0b00000111;
	ADMUX |=(1<<REFS0);
	ADMUX &= (0xF0);
	ADMUX |= (ch & 0x0F);
	_delay_ms(5);
	ADCSRA|=(1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	//ADCSRA|=(1<<ADIF);

	return(ADC);
}

typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[6];
const unsigned char tasksNum = 6;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodJStck1 = 100;
const unsigned long periodJStck2 = 100;
const unsigned long periodDisplay = 100;
const unsigned long periodCollision = 50;
unsigned long periodBall = 400;
unsigned long periodBall2 = 550;
unsigned char bX = 42;
 unsigned char bY = 24;
 unsigned char bX2 = 42;
 unsigned char bY2 = 24;
unsigned char bar1 = 42; 
unsigned char bar2 = 42;
 unsigned char collision = 0;
 unsigned char collision2 = 0;
unsigned char score1 = 0;
 unsigned int score2 = 0;
unsigned int scored = 0;
unsigned char p1;
unsigned char p2;
unsigned char tTotal;
unsigned char ball2On = 0;
unsigned char ready1 =0;
unsigned char ready2 =0;
unsigned char gameover = 0;
unsigned char t;
unsigned char t1;

enum J1_States { J1_SMStart, J1_wait, J1_left, J1_right }; unsigned char L1; unsigned char R1;
int TickFct_jStck1(int state);
enum J2_States { J2_SMStart, J2_wait, J2_left, J2_right }; unsigned char L2; unsigned char R2;
int TickFct_jStck2(int state);
enum BC_States { BC_SMStart, BC_wait, BC_s1, BC_s2, BC_s3, BC_s4, BC_s5, BC_s6, BC_s7, BC_s8};
int TickFct_ballComp(int state);
enum BC2_States { BC2_SMStart, BC2_wait, BC2_s1, BC2_s2, BC2_s3, BC2_s4, BC2_s5, BC2_s6, BC2_s7, BC2_s8};
int TickFct_ballComp2(int state);
enum CO_States { CO_SMStart, CO_wait, CO_1, CO_2, CO_3, CO_4, CO_score1, CO_score2, CO_score11, CO_score22};
int TickFct_collision(int state);
enum DI_States { DI_SMStart, DI_wait, DI_display,DI_displayflash };
int TickFct_display(int state);

int main() {
	
	
	DDRB = 0xFF;	PORTB = 0x00;
	DDRD = 0xFF;	PORTD = 0x00;
	
	
	
	
	unsigned char i=0;
	
	tasks[i].state = DI_SMStart;
	tasks[i].period = periodDisplay;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_display;
	++i;
	tasks[i].state = J1_SMStart;
	tasks[i].period = periodJStck1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_jStck1;
	++i;
	tasks[i].state = J2_SMStart;
	tasks[i].period = periodJStck2;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_jStck2;
	++i;
	tasks[i].state = BC_SMStart;
	tasks[i].period = periodBall;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_ballComp;
	++i;
	tasks[i].state = BC2_SMStart;
	tasks[i].period = periodBall2;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_ballComp2;
	++i;
	tasks[i].state = CO_SMStart;
	tasks[i].period = periodCollision;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_collision;
	
	
	//Initialize LCD Module
	LCD_init();
	//Initialize ADC Peripheral
	ADC_init();
	
	nokia_lcd_init();
	nokia_lcd_clear();
	nokia_lcd_write_string("Game starts in 3",1);
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_render();
	delay_ms(1000);
	
	nokia_lcd_clear();
	nokia_lcd_write_string("Game starts in 2",1);
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_render();
	delay_ms(1000);
	
	nokia_lcd_clear();
	nokia_lcd_write_string("Game starts in 1",1);
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_render();
	delay_ms(1000);
	
	nokia_lcd_clear();
	nokia_lcd_write_string("BEGIN!",2);
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_render();
	delay_ms(1000);
	unsigned char j=0;
	
	while(1) {
		
		x = ReadADC(1);
		x = x -515;
		x1 = ReadADC(3);
		x1 = x1 -515;
		
		unsigned char i;
		for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
			TimerSet(tasksPeriodGCD);
			TimerOn();
			if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				tasks[i].elapsedTime = 0;
			}
			tasks[i].elapsedTime += tasksPeriodGCD;
		}
		
	}
	
	return 0;
}




int TickFct_jStck1(int state) {
	switch(state) { // Transitions
		case J1_SMStart: // Initial transition
		
		state = J1_wait;
		
		break;
		case J1_wait:
		if (x < -100 && bar1 > 5){
			state = J1_left;
		}
		else if (x > 100 && bar1 < 79){
			state = J1_right;
		}
		break;
		case J1_left:
		if (!(x < -100)){
			state = J1_wait;
		}
		break;
		case J1_right:
		if (!(x > 100)){
			state = J1_wait;
		}
		break;
		default:
		state = J1_SMStart;
	} // Transitions

	switch(state) { // State actions
		case J1_SMStart: // Initial transition
		L1 = 0;
		R1 = 0;
		break;
		case J1_wait:
		
		break;
		case J1_left:
		if(bar1 !=5){
			bar1--;
		}
		break;
		case J1_right:
		if(bar1 !=79){
			bar1++;
		}
		break;
		default:
		break;
	} // State actions
	return state;
}

int TickFct_jStck2(int state) {
	switch(state) { // Transitions
		case J2_SMStart: // Initial transition
		
		state = J2_wait;
		
		break;
		case J2_wait:
		if (x1 < -100 && bar2 > 5){
			state = J1_left;
		}
		else if (x1 > 100 && bar2 < 79){
			state = J2_right;
		}
		break;
		case J2_left:
		if (!(x1 < -100) ){
			state = J2_wait;
		}
		break;
		case J2_right:
		if (!(x1 > 100)){
			state = J2_wait;
		}
		break;
		default:
		state = J2_SMStart;
	} // Transitions

	switch(state) { // State actions
		case J2_SMStart: // Initial transition
		L1 = 0;
		R1 = 0;
		break;
		case J2_wait:
		
		break;
		case J2_left:
		if(bar2 !=5){
			bar2--;
		}
		break;
		case J2_right:
		if(bar2 !=79){
			bar2++;
		}
		break;
		default:
		break;
	} // State actions
	return state;
}

int TickFct_ballComp(int state) {
	unsigned char t;
	unsigned char cT;
	switch(state) { // Transitions
		case BC_SMStart: // Initial transition
		if(x >100 || x < -100){
			ready1 = 0;
			nokia_lcd_clear();
			nokia_lcd_write_string("ready p1",1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
		}
		if(x1 >100 || x1 < -100){
			ready2 = 0;
			nokia_lcd_write_string("ready p2",1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
		}
		if (ready1 && ready2)
		{
			state = BC_wait;
		}
		break;
		case BC_wait:
		t = (rand() % 8) +1;
		if(t == 1){
			state = BC_s1;
		}
		if(t == 2){
			state = BC_s2;
		}
		if(t == 3){
			state = BC_s3;
		}
		if(t == 4){
			state = BC_s4;
		}
		if(t == 5){
			state = BC_s5;
		}
		if(t == 6){
			state = BC_s6;
		}
		if(t == 7){
			state = BC_s7;
		}
		if(t == 8){
			state = BC_s8;
		}
		break;
		case BC_s1:
		if (scored)
		{
			state = BC_wait;
		}
		
		/*nokia_lcd_clear();
		nokia_lcd_write_string("1",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(200);*/
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s7;
			}
			if(cT == 2){
				state = BC_s8;
			}
		}
		if(collision == 2){
		/*nokia_lcd_clear();
		nokia_lcd_write_string("2",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(200);*/
			state = BC_s3;
		}
		
		else if (collision == 0){
		/*nokia_lcd_clear();
		nokia_lcd_write_string("3",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(200);*/
			state = BC_s1;
		}
		collision = 0;
		break;
		case BC_s2:
		if (scored)
		{
			state = BC_wait;
		}
		
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s7;
			}
			if(cT == 2){
				state = BC_s8;
			}
		}
		if(collision == 2){
			state = BC_s3;
		}
			//state = BC_wait;
		else if (collision == 0){
			state = BC_s2;
		}
		
		collision = 0;
		break;
		case BC_s3:
		if (scored)
		{
			state = BC_wait;
		}
		
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s5;
			}
			if(cT == 2){
				state = BC_s6;
			}
		}
		if(collision == 2){
			state = BC_s2;
		}
		
		else if (collision == 0){
			state = BC_s3;
		}
		collision = 0;
		break;
		case BC_s4:
		if (scored)
		{
			state = BC_wait;
		}
		
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s5;
			}
			if(cT == 2){
				state = BC_s6;
			}
		}
		if(collision == 2){
			state = BC_s1;
		}
		
		else if (collision == 0){
			state = BC_s4;
		}
		collision = 0;
		break;
		case BC_s5:
		if (scored)
		{
			state = BC_wait;
		}
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s4;
			}
			if(cT == 2){
				state = BC_s3;
			}
		}
		if(collision == 2){
			state = BC_s8;
		}
		
		else if (collision == 0){
			state = BC_s5;
		}
		collision = 0;
		break;
		case BC_s6:
		if (scored)
		{
			state = BC_wait;
		}
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s3;
			}
			if(cT == 2){
				state = BC_s4;
			}
		}
		if(collision == 2){
			state = BC_s7;
		}
		if (scored)
		{
			state = BC_wait;
		}
		
		
		else if (collision == 0){
			state = BC_s6;
		}
		
		   
		collision = 0;
		break;
		case BC_s7:
		if (scored)
		{
			state = BC_wait;
		}
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s1;
			}
			if(cT == 2){
				state = BC_s2;
			}
		}
		if(collision == 2){
			state = BC_s6;
		}
		if (scored)
		{
			state = BC_wait;
		}
		
		else if (collision == 0){
			state = BC_s7;
		}
		collision = 0;
		break;
		case BC_s8:
		if (scored)
		{
			state = BC_wait;
		}
		if(collision == 1){
			cT = (rand() % 2) +1;
			if(cT == 1){
				state = BC_s1;
			}
			if(cT == 2){
				state = BC_s2;
			}
		}
		if(collision == 2){
			state = BC_s5;
		}
			//state = BC_wait;
		else if (collision == 0){
			state = BC_s8;
		}
		
		
		collision = 0;
		break;
		default:
		state = BC_SMStart;
	} // Transitions

	switch(state) { // State actions
		case BC_SMStart: // Initial transition
		state = BC_wait;
		break;
		case BC_wait:
		break;
		case BC_s1:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("here",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX--;
		bY++;
		break;
		case BC_s2:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("2",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX--;
		bY+=2;
		break;
		case BC_s3:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("3",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX++;
		bY+=2;
		break;
		case BC_s4:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("4",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX++;
		bY++;
		break;
		case BC_s5:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("5",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX++;
		bY--;
		break;
		case BC_s6:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("6",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX++;
		bY-=2;
		break;
		case BC_s7:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("7",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(100);*/
		bX--;
		bY-=2;
		break;
		case BC_s8:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("8",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX--;
		bY--;
		break;
		default:
		break;
	} // State actions
	return state;
}

int TickFct_ballComp2(int state) {
	unsigned char t1;
	unsigned char cT1;
	switch(state) { // Transitions
		case BC2_SMStart: // Initial transition
		if(x >100 || x < -100){
			ready1 = 0;
			nokia_lcd_clear();
			nokia_lcd_write_string("ready p1",1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
		}
		if(x1 >100 || x1 < -100){
			ready2 = 0;
			nokia_lcd_write_string("ready p2",1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
		}
		if (ready1 && ready2)
		{
			state = BC2_wait;
		}
		
		break;
		case BC2_wait:
		t1= (rand() % 8) +1;
		if(t1== 1){
			state = BC2_s1;
		}
		if(t1== 2){
			state = BC2_s2;
		}
		if(t1== 3){
			state = BC2_s3;
		}
		if(t1== 4){
			state = BC2_s4;
		}
		if(t1== 5){
			state = BC2_s5;
		}
		if(t1== 6){
			state = BC2_s6;
		}
		if(t1== 7){
			state = BC2_s7;
		}
		if(t1== 8){
			state = BC2_s8;
		}
		break;
		case BC2_s1:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s7;
			}
			if(cT1== 2){
				state = BC2_s8;
			}
		}
		if(collision2 == 2){
			state = BC2_s3;
		}
			//state = BC_wait;
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		else if (collision2 == 0){
			state = BC2_s1;
		}
		collision2 = 0;
		break;
		case BC2_s2:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s7;
			}
			if(cT1== 2){
				state = BC2_s8;
			}
		}
		if(collision2 == 2){
			state = BC2_s3;
		}
			//state = BC_wait;
		else if (collision2 == 0){
			state = BC2_s2;
		}
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		collision2 = 0;
		break;
		case BC2_s3:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s5;
			}
			if(cT1== 2){
				state = BC2_s6;
			}
		}
		if(collision2 == 2){
			state = BC2_s2;
		}
			//state = BC_wait;
		else if (collision2 == 0){
			state = BC2_s3;
		}
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		collision2 = 0;
		break;
		case BC2_s4:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s5;
			}
			if(cT1== 2){
				state = BC2_s6;
			}
		}
		if(collision2 == 2){
			state = BC2_s1;
		}
			//state = BC_wait;
		else if (collision2 == 0){
			state = BC2_s4;
		}
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		collision2 = 0;
		break;
		case BC2_s5:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s4;
			}
			if(cT1== 2){
				state = BC2_s3;
			}
		}
		if(collision2 == 2){
			state = BC2_s8;
		}
			//state = BC_wait;
		else if (collision2 == 0){
			state = BC2_s5;
		}
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		collision2 = 0;
		break;
		case BC2_s6:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s3;
			}
			if(cT1== 2){
				state = BC2_s4;
			}
		}
		if(collision2 == 2){
			state = BC2_s7;
		}
			//state = BC_wait;
		
		else if (collision2 == 0){
			state = BC2_s6;
		}
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		collision2 = 0;
		break;
		case BC2_s7:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s1;
			}
			if(cT1== 2){
				state = BC2_s2;
			}
		}
		if(collision2 == 2){
			state = BC2_s6;
		}
			//state = BC_wait;
		else if (collision2 == 0){
			state = BC2_s7;
		}
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		collision2 = 0;
		break;
		case BC2_s8:
		if(collision2 == 1){
			cT1= (rand() % 2) +1;
			if(cT1== 1){
				state = BC2_s1;
			}
			if(cT1== 2){
				state = BC2_s2;
			}
		}
		if(collision2 == 2){
			state = BC2_s5;
		}
			//state = BC_wait;
		else if (collision2 == 0){
			state = BC2_s8;
		}
		if(t = t1 && collision2 ==0){
			if(t1%2){
				t1--;
			}
			else{
				t1++;
			}
		}
		collision2 = 0;
		break;
		default:
		state = BC2_SMStart;
	} // Transitions

	switch(state) { // State actions
		case BC2_SMStart: // Initial transition
		state = BC2_wait;
		break;
		case BC2_wait:
		break;
		case BC2_s1:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("1",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX2--;
		bY2++;
		break;
		case BC2_s2:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("2",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX2--;
		bY2+=2;
		break;
		case BC2_s3:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("3",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX2++;
		bY2+=2;
		break;
		case BC2_s4:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("4",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX2++;
		bY2++;
		break;
		case BC2_s5:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("5",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX2++;
		bY2--;
		break;
		case BC2_s6:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("6",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX2++;
		bY2-=2;
		break;
		case BC2_s7:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("7",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(100);*/
		bX2--;
		bY2-=2;
		break;
		case BC2_s8:
		/*nokia_lcd_clear();
		nokia_lcd_write_string("8",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_render();
		delay_ms(20);*/
		bX2--;
		bY2--;
		break;
		default:
		break;
	} // State actions
	return state;
}


int TickFct_collision(int state) {
	switch(state) { // Transitions
		case CO_SMStart: // Initial transition
		state = CO_wait;
		
		break;
		case CO_wait:
		
		if((bX == bar2 || bX == bar2-1 || bX == bar2-2 || bX == bar2-3 || bX == bar2-4 || bX == bar2-5 || bX == bar2+1 || bX == bar2+2 || bX == bar2+3 || bX == bar2+4 || bX == bar2+5 || bX-1 == bar2 || bX-1 == bar2-1 || bX-1 == bar2-2 || bX-1 == bar2-3 || bX-1 == bar2-4 || bX-1 == bar2-5 || bX-1 == bar2+1 || bX-1 == bar2+2 || bX-1 == bar2+3 || bX-1 == bar2+4 || bX-1 == bar2+5 || bX+1 == bar2 || bX+1 == bar2-1 || bX+1 == bar2-2 || bX+1 == bar2-3 || bX+1 == bar2-4 || bX+1 == bar2-5 || bX+1 == bar2+1 || bX+1 == bar2+2 || bX+1 == bar2+3 || bX+1 == bar2+4 || bX+1 == bar2+5 )&& bY == 44){
			//if ( bY == 40){
			state = CO_1;
			
			//}
		}
		
		if((bX == bar1 || bX == bar1-1 || bX == bar1-2 || bX == bar1-3 || bX == bar1-4 || bX == bar1-5 || bX == bar1+1 || bX == bar1+2 || bX == bar1+3 || bX == bar1+4 || bX == bar1+5 || bX-1 == bar1 || bX-1 == bar1-1 || bX-1 == bar1-2 || bX-1 == bar1-3 || bX-1 == bar1-4 || bX-1 == bar1-5 || bX-1 == bar1+1 || bX-1 == bar1+2 || bX-1 == bar1+3 || bX-1 == bar1+4 || bX-1 == bar1+5 || bX+1 == bar1 || bX+1 == bar1-1 || bX+1 == bar1-2 || bX+1 == bar1-3 || bX+1 == bar1-4 || bX+1 == bar1-5 || bX+1 == bar1+1 || bX+1 == bar1+2 || bX+1 == bar1+3 || bX+1 == bar1+4 || bX+1 == bar1+5) && bY == 2){
			//if ( bY == 2){
			state = CO_1;
			//}
		}
		
		if((bX2 == bar2 || bX2 == bar2-1 || bX2 == bar2-2 || bX2 == bar2-3 || bX2 == bar2-4 || bX2 == bar2-5 || bX2 == bar2+1 || bX2 == bar2+2 || bX2 == bar2+3 || bX2 == bar2+4 || bX2 == bar2+5 || bX2-1 == bar2 || bX2-1 == bar2-1 || bX2-1 == bar2-2 || bX2-1 == bar2-3 || bX2-1 == bar2-4 || bX2-1 == bar2-5 || bX2-1 == bar2+1 || bX2-1 == bar2+2 || bX2-1 == bar2+3 || bX2-1 == bar2+4 || bX2-1 == bar2+5 || bX2+1 == bar2 || bX2+1 == bar2-1 || bX2+1 == bar2-2 || bX2+1 == bar2-3 || bX2+1 == bar2-4 || bX2+1 == bar2-5 || bX2+1 == bar2+1 || bX2+1 == bar2+2 || bX2+1 == bar2+3 || bX2+1 == bar2+4 || bX2+1 == bar2+5 )&& bY2 == 44){
			if (( score1 >= 4 || score2 >= 4) ){
				state = CO_3;
			}
		}
		
		if((bX2 == bar1 || bX2 == bar1-1 || bX2 == bar1-2 || bX2 == bar1-3 || bX2 == bar1-4 || bX2 == bar1-5 || bX2 == bar1+1 || bX2 == bar1+2 || bX2 == bar1+3 || bX2 == bar1+4 || bX2 == bar1+5 || bX2-1 == bar1 || bX2-1 == bar1-1 || bX2-1 == bar1-2 || bX2-1 == bar1-3 || bX2-1 == bar1-4 || bX2-1 == bar1-5 || bX2-1 == bar1+1 || bX2-1 == bar1+2 || bX2-1 == bar1+3 || bX2-1 == bar1+4 || bX2-1 == bar1+5 || bX2+1 == bar1 || bX2+1 == bar1-1 || bX2+1 == bar1-2 || bX2+1 == bar1-3 || bX2+1 == bar1-4 || bX2+1 == bar1-5 || bX2+1 == bar1+1 || bX2+1 == bar1+2 || bX2+1 == bar1+3 || bX2+1 == bar1+4 || bX2+1 == bar1+5) && bY2 == 2){
			if ((score1 >= 4 || score2 >= 4) ){
				state = CO_3;
			}
		}
		
		
		
		if(bX-1 == 2 || bX+1 == 82){
			state = CO_2;
		}
		
		if((bX2-1 == 1 || bX2+1 == 83)&& ((score1 >= 4 || score2 >= 4))){
			state = CO_4;
		}
		
		if(bY < 1){
			state = CO_score1;
			
		}
		
		if(bY2 < 1 && ((score1 >= 4 || score2 >= 4))){
			state = CO_score11;
			
		}
		
		if(bY > 46){
			state = CO_score2;
			
		}
		
		if(bY2 > 46 && ((score1 >= 4 || score2 >= 4) )){
			state = CO_score22;
			
		}
		break;
		case CO_1:
		state = CO_wait;
		break;
		case CO_2:
		state = CO_wait;
		break;
		case CO_3:
		state = CO_wait;
		break;
		case CO_4:
		state = CO_wait;
		break;
		case CO_score1:
		state = CO_wait;
		break;
		case CO_score2:
		state = CO_wait;
		break;
		case CO_score11:
		state = CO_wait;
		break;
		case CO_score22:
		state = CO_wait;
		break;
		default:
		state = CO_SMStart;
	} // Transitions

	switch(state) { // State actions
		case CO_SMStart: // Initial transition
		break;
		case CO_wait:
		break;
		case CO_1:
		collision = 1;
		break;
		case CO_2:
		collision = 2;
		break;
		case CO_3:
		collision2 = 1;
		break;
		case CO_4:
		collision2 = 2;
		break;
		case CO_score1:
		score2++;
		scored = 1;
		p1 = 1;
		break;
		case CO_score2:
		score1++;
		scored = 1;
		p2 = 1;
		break;
		case CO_score11:
		score2++;
		scored = 1;
		p1 = 1;
		break;
		case CO_score22:
		score1++;
		scored = 1;
		p2 = 1;
		break;
		default:
		break;
	} // State actions
	return state;
}


int TickFct_display(int state) {
	unsigned char time1;
	switch(state) { // Transitions
		case DI_SMStart: // Initial transition
		state = DI_wait;
		break;
		case DI_wait:
		state = DI_display;
		break;
		case DI_display:
		if (score1 < 6 && score2 < 6){
			
			state = DI_wait;
		}
		if(score2 >= 6 && score1 >= 6 && score1 != 8 && score2 != 8){
			state = DI_displayflash;
		}
		state = DI_wait;
		break;
		case DI_displayflash:
		state = DI_display;
		break;
		default:
			state = DI_SMStart;
	} // Transitions

	switch(state) { // State actions
		case DI_SMStart: // Initial transition
		break;
		case DI_wait:
		break;
		//set ball movements
		
		case DI_display:
		
		if (score1>=8 || score2 >=8)
		{
			if (score1 == 8)
			{
				nokia_lcd_clear();
				nokia_lcd_write_string("Player 1 wins",1);
				nokia_lcd_set_cursor(0, 10);
				nokia_lcd_render();
				delay_ms(1000);
			}
			if (score2 == 8)
			{
				nokia_lcd_clear();
				nokia_lcd_write_string("Player 2 wins",1);
				nokia_lcd_set_cursor(0, 10);
				nokia_lcd_render();
				delay_ms(1000);
			}
			scored = 0;
			gameover = 1;
			
	}
		
		else if(scored == 0){
			nokia_lcd_clear();
			nokia_lcd_set_pixel(bar1,1,1);
			nokia_lcd_set_pixel(bar1-1,1,1);
			nokia_lcd_set_pixel(bar1-2,1,1);
			nokia_lcd_set_pixel(bar1-3,1,1);
			nokia_lcd_set_pixel(bar1+1,1,1);
			nokia_lcd_set_pixel(bar1+2,1,1);
			nokia_lcd_set_pixel(bar1+3,1,1);
			nokia_lcd_set_pixel(bar1-4,1,1);
			nokia_lcd_set_pixel(bar1+4,1,1);
			nokia_lcd_set_pixel(bar1+5,1,1);
			nokia_lcd_set_pixel(bar1-5,1,1);
			nokia_lcd_set_pixel(bar2,47,1);
			nokia_lcd_set_pixel(bar2-1,47,1);
			nokia_lcd_set_pixel(bar2-2,47,1);
			nokia_lcd_set_pixel(bar2-3,47,1);
			nokia_lcd_set_pixel(bar2+1,47,1);
			nokia_lcd_set_pixel(bar2+2,47,1);
			nokia_lcd_set_pixel(bar2+3,47,1);
			nokia_lcd_set_pixel(bar2-4,47,1);
			nokia_lcd_set_pixel(bar2+4,47,1);
			nokia_lcd_set_pixel(bar2-5,47,1);
			nokia_lcd_set_pixel(bar2+5,47,1);
			nokia_lcd_set_pixel(bX,bY,1);
			nokia_lcd_set_pixel(bX-1,bY+1,1);
			nokia_lcd_set_pixel(bX,bY+1,1);
			nokia_lcd_set_pixel(bX+1,bY+1,1);
			nokia_lcd_set_pixel(bX+1,bY,1);
			nokia_lcd_set_pixel(bX+1,bY-1,1);
			nokia_lcd_set_pixel(bX,bY-1,1);
			nokia_lcd_set_pixel(bX-1,bY-1,1);
			nokia_lcd_set_pixel(bX-1,bY,1);
			
			
			if (((score1 == 4 || score2 == 4) && scored) )
			{
				bX2 =42;
				bY2 = 24;
			}
			if ((score1 >= 4 || score2 >= 4) )
			{
				
				nokia_lcd_set_pixel(bX2,bY2,1);
				nokia_lcd_set_pixel(bX2-1,bY2+1,1);
				nokia_lcd_set_pixel(bX2,bY2+1,1);
				nokia_lcd_set_pixel(bX2+1,bY2+1,1);
				nokia_lcd_set_pixel(bX2+1,bY2,1);
				nokia_lcd_set_pixel(bX2+1,bY2-1,1);
				nokia_lcd_set_pixel(bX2,bY2-1,1);
				nokia_lcd_set_pixel(bX2-1,bY2-1,1);
				nokia_lcd_set_pixel(bX2-1,bY2,1);
				
				
			}
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
			
			
		}
		else if(scored == 1 && !gameover){
			
			if(p1){
				nokia_lcd_clear();
				nokia_lcd_write_string("Player 2 has scored",1);
				nokia_lcd_set_cursor(0, 10);
				nokia_lcd_render();
				delay_ms(3000);
				p1 = 0;
			}
			else if (p2){
				nokia_lcd_clear();
				nokia_lcd_write_string("Player 1 has scored",1);
				nokia_lcd_set_cursor(0, 10);
				nokia_lcd_render();
				delay_ms(3000);
				p2 = 0;
			}
			
			unsigned char t1[10];
			unsigned char t2[10];
			unsigned char t[10];
			unsigned char j[10];
			itoa(score1, t1, 10);
			itoa(score2, t2, 10);
			strcat(t1, " : ");
			strcat(t1,t2);
			nokia_lcd_clear();
			nokia_lcd_write_string(t1 ,1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
			delay_ms(1000);
			
			
			nokia_lcd_clear();
			nokia_lcd_write_string("Ready",1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
			delay_ms(1000);
			
			nokia_lcd_clear();
			nokia_lcd_write_string("Set",1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
			delay_ms(1000);
			
			nokia_lcd_clear();
			nokia_lcd_write_string("Play",1);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
			delay_ms(1000);
			
		
			bar1 = 42;
			bar2 = 42;
			bX = 42;
			bY = 24;
			bX2 = 42;
			bY2 = 24;
			scored = 0;
		}
		//LCD_DisplayString("SCORE1:)
		break;
		
		case DI_displayflash:
			nokia_lcd_clear();
			nokia_lcd_set_pixel(bar1,1,1);
			nokia_lcd_set_pixel(bar1-1,1,1);
			nokia_lcd_set_pixel(bar1-2,1,1);
			nokia_lcd_set_pixel(bar1-3,1,1);
			nokia_lcd_set_pixel(bar1+1,1,1);
			nokia_lcd_set_pixel(bar1+2,1,1);
			nokia_lcd_set_pixel(bar1+3,1,1);
			nokia_lcd_set_pixel(bar1-4,1,1);
			nokia_lcd_set_pixel(bar1+4,1,1);
			nokia_lcd_set_pixel(bar1+5,1,1);
			nokia_lcd_set_pixel(bar1-5,1,1);
			nokia_lcd_set_pixel(bar2,47,1);
			nokia_lcd_set_pixel(bar2-1,47,1);
			nokia_lcd_set_pixel(bar2-2,47,1);
			nokia_lcd_set_pixel(bar2-3,47,1);
			nokia_lcd_set_pixel(bar2+1,47,1);
			nokia_lcd_set_pixel(bar2+2,47,1);
			nokia_lcd_set_pixel(bar2+3,47,1);
			nokia_lcd_set_pixel(bar2-4,47,1);
			nokia_lcd_set_pixel(bar2+4,47,1);
			nokia_lcd_set_pixel(bar2-5,47,1);
			nokia_lcd_set_pixel(bar2+5,47,1);
			nokia_lcd_set_pixel(bX,bY,0);
			nokia_lcd_set_pixel(bX-1,bY+1,0);
			nokia_lcd_set_pixel(bX,bY+1,0);
			nokia_lcd_set_pixel(bX+1,bY+1,0);
			nokia_lcd_set_pixel(bX+1,bY,0);
			nokia_lcd_set_pixel(bX+1,bY-1,0);
			nokia_lcd_set_pixel(bX,bY-1,0);
			nokia_lcd_set_pixel(bX-1,bY-1,0);
			nokia_lcd_set_pixel(bX-1,bY,0);
			nokia_lcd_set_pixel(bX2,bY2,0);
			nokia_lcd_set_pixel(bX2-1,bY2+1,0);
			nokia_lcd_set_pixel(bX2,bY2+1,0);
			nokia_lcd_set_pixel(bX2+1,bY2+1,0);
			nokia_lcd_set_pixel(bX2+1,bY2,0);
			nokia_lcd_set_pixel(bX2+1,bY2-1,0);
			nokia_lcd_set_pixel(bX2,bY2-1,0);
			nokia_lcd_set_pixel(bX2-1,bY2-1,0);
			nokia_lcd_set_pixel(bX2-1,bY2,0);
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_render();
			delay_ms(50);
		break;
		default:
		break;
	} // State actions
	return state;
}
