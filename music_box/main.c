#include "avr.h"
#include "lcd.h"
#include <stdlib.h>


#define A 114
#define AS 107
#define B 101
#define C 96
#define CS 90
#define D 85
#define DS 80
#define E 76
#define F 72
#define FS 68
#define G 64
#define GS 60
#define A2 57
#define AS2 54
#define B2 51
#define B2SLIDE 52
#define C2 48
#define CS2 45
#define D2 43
#define DS2 40
#define E2 38
#define F2 36
#define FS2 34
#define G2 32
#define GS2 30
#define A3 28
#define AS3 27
#define B3 25
#define C3 24
#define CS3 23
#define D3 21
#define DS3 20
#define E3 19
#define F3 18
#define FS3 17
#define G3 16
#define GS3 15


// tune our instrument. DEFAULT STD TUNING = 1.25
const float MODIFIER = 1.25;


unsigned char LETSGO = 19;
unsigned char SLOWDOWN = 20;
unsigned char RESET = 21;

unsigned int tempo;
int * quarterNote;
int * halfNote;
int * wholeNote;
int * eighthNote;
int * sixteenNote;
int * dottedEight;
int * dottedSixteenth;
int * dottedQuarter;
int * dottedHalf;


struct note {
	unsigned char freq;
	int * duration;
};


unsigned char pressed(unsigned char r, unsigned char c)
{
	DDRC = 0x0F;
	PORTC = 0xff;
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	wait_avr(1);
	
	if(GET_BIT(PINC, c+4))
	{
		return 0;
	}
	return 1;
}

unsigned char get_key()
{
	unsigned char r,c;
	for(r=0;r<4;r++)
	{
		for(c=0;c<4;c++)
		{
			if(pressed(r,c))
			{
				return (r*4)+c+1;
			}
		}
	}
	return 0;
}

unsigned char parse_key(unsigned char in)
{
	if (in <= 3 && in > 0)
	return in;
	if (in > 4 && in < 8)
	return in-1;
	if (in > 8 && in < 12)
	return in-2;
	if (in == 14)
	return 0;
	if (in == 12)
	return RESET;
	if (in == 4)
	return LETSGO;
	if (in == 8)
	return SLOWDOWN;
	return 200;
}

unsigned char get_actual_key() {
	unsigned char key;
	key = parse_key(get_key());

	if (key >= 0 && key <= 22)
	{
		return key;
	}
	return 200;
}
	
void play_note(unsigned char freq, int * duration) {
	TCNT1 = 0;
	unsigned char volume = 100;
	int newfreq = freq * MODIFIER;
	change_tempo();
	
	if (freq == B2SLIDE) {
		for (;;) {
			SET_BIT(PORTA, 0);
			wait_avr(newfreq);
			CLR_BIT(PORTA, 0);
			wait_avr(newfreq);
			
			if(TCNT1 >= *duration)
				break;
			if (newfreq < (B*MODIFIER))
				++newfreq;
			else
				newfreq = B*MODIFIER;
			
		}
	} else {
		unsigned char flag = 0;

		if (freq == A3)
			flag = 1;
		//unsigned char user_input;

		for (;;) {	
			SET_BIT(PORTA, 0);
			wait_avr(newfreq);
			CLR_BIT(PORTA, 0);
			wait_avr(newfreq);
		
			// tune dat A3 LOL
			if (flag)
				wait_avr(1);
			
			if(TCNT1 >= *duration)
				break;
			
		}
	}
	
	wait_avr(2000);
}

void play_music(struct note song[], int n) {
	int i;
	for (i = 0; i < n; i++) {
		play_note(song[i].freq, song[i].duration);
		
	}
}

void ini_music(void)
{
	ini_avr();
	SET_BIT(DDRA, 0); // previously in avr.c
	SET_BIT(TCCR1B, 0);
	SET_BIT(TCCR1B, 2);
}

void change_tempo(void) {
	* quarterNote =  (8000000/(tempo/60) -1)/1024;
	*halfNote = * quarterNote*2;
	*wholeNote = * quarterNote*4;
	*eighthNote = * quarterNote/2;
	*sixteenNote = * quarterNote/4;
	*dottedEight = * eighthNote*3;
	*dottedSixteenth = (* sixteenNote * 3)/2;
	*dottedQuarter = * eighthNote * 3;
	*dottedHalf = * quarterNote*3;
	
	// ((1 / Target Frequency) / (1 / Timer Clock Frequency))-1
}

void ini_notes(void) {
	quarterNote =  (int *)malloc(sizeof(int));
	halfNote = (int *)malloc(sizeof(int));
	wholeNote = (int *)malloc(sizeof(int));
	eighthNote = (int *)malloc(sizeof(int));
	sixteenNote = (int *)malloc(sizeof(int));
	dottedEight = (int *)malloc(sizeof(int));
	dottedSixteenth = (int *)malloc(sizeof(int));
	dottedQuarter = (int *)malloc(sizeof(int));
	dottedHalf = (int *)malloc(sizeof(int));
}

int main()
{
	ini_notes();
	ini_music();
	ini_lcd();
	
	unsigned char user_input;
	struct note canon_in_d[] = {
		{A3, eighthNote},
		{FS2, sixteenNote},
		{G2, sixteenNote},
		{A3, eighthNote},
		{FS2, sixteenNote},
		{G2, sixteenNote},
		{A3, sixteenNote},
		{A2, sixteenNote},
		{B2, sixteenNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		{E2, sixteenNote},
		{FS2, sixteenNote},
		{G2, sixteenNote},
		{FS2, eighthNote},
		{D2, sixteenNote},
		{E2, sixteenNote},
		{FS2, eighthNote},
		{FS, sixteenNote},
		{G, sixteenNote},
		{A2, sixteenNote},
		{B2, sixteenNote},
		{A2, sixteenNote},
		{G, sixteenNote},
		{A2, sixteenNote},
		{G, sixteenNote},
		{A2, sixteenNote},
		{FS, sixteenNote},
		{G, sixteenNote},
		{A2, sixteenNote},
		{G, eighthNote},
		{B2, sixteenNote},
		{A2, sixteenNote},
		{G, eighthNote},
		{FS, sixteenNote},
		{E, sixteenNote},
		{FS, sixteenNote},
		{E, sixteenNote},
		{D, sixteenNote},
		{E, sixteenNote},
		{FS, sixteenNote},
		{G, sixteenNote},
		{A2, sixteenNote},
		{B2, sixteenNote},
		{G, eighthNote},
		{B2, sixteenNote},
		{A2, sixteenNote},
		{B2, eighthNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		{A2, sixteenNote},
		{B2, sixteenNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		{E2, sixteenNote},
		{FS2, sixteenNote},
		{G2, sixteenNote},
		{A3, sixteenNote},
		
		// measure 21
		{FS2, eighthNote},
		{D2, sixteenNote},
		{E2, sixteenNote},
		{FS2, eighthNote},
		{E2, sixteenNote},
		{D2, sixteenNote},
		{E2, sixteenNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		{E2, sixteenNote},
		{FS2, sixteenNote},
		{E2, sixteenNote},
		{D2, sixteenNote},
		{CS2, sixteenNote},
		{D2, eighthNote},
		{B2, sixteenNote},
		{CS2, sixteenNote},
		{D2, eighthNote},
		{D, sixteenNote},
		{E, sixteenNote},
		{FS, sixteenNote},
		{G, sixteenNote},
		{FS, sixteenNote},
		{E, sixteenNote},
		{FS, sixteenNote},
		{D2, sixteenNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		
		// measure 22
		{B2, eighthNote},
		{D2, sixteenNote},
		{CS2, sixteenNote},
		{B2, eighthNote},
		{A2, sixteenNote},
		{G, sixteenNote},
		{A2, sixteenNote},
		{G, sixteenNote},
		{FS, sixteenNote},
		{G, sixteenNote},
		{A2, sixteenNote},
		{B2, sixteenNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		{B2, eighthNote},
		{D2, sixteenNote},
		{CS2, sixteenNote},
		{D2, eighthNote},
		{CS2, sixteenNote},
		{B2, sixteenNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		{E2, sixteenNote},
		{D2, sixteenNote},
		{CS2, sixteenNote},
		{D2, sixteenNote},
		{B2, sixteenNote},
		{CS2, sixteenNote},
		{D2, halfNote},

	};	
	struct note beautiful_now[] = {
		// 1
		{B, eighthNote},
		{B, eighthNote},
		{D, eighthNote},
		{B, eighthNote},
		{G, quarterNote},
		
		// 2
		{FS, quarterNote},
		{B, eighthNote},
		{B, eighthNote},
		{D, eighthNote},
		{B, eighthNote},
		{G, quarterNote},
		
		// 3
		{FS, quarterNote},
		{B, eighthNote},
		{B, eighthNote},
		{D, eighthNote},
		{B, eighthNote},
		{A2, quarterNote},
		
		// 4
		{FS, quarterNote},
		{D, eighthNote},
		{CS, eighthNote},
		{D, eighthNote},
		{CS, eighthNote},
		{A2, quarterNote},
			
		// 5
		{FS, quarterNote},
		{B, eighthNote},
		{B, eighthNote},
		{D, eighthNote},
		{B, eighthNote},
		{G, quarterNote},
			
		// 6
		{FS, quarterNote},
		{B, eighthNote},
		{B, eighthNote},
		{D, eighthNote},
		{B, eighthNote},
		{G, quarterNote},
			
		// 7
		{FS, quarterNote},
		{B, eighthNote},
		{B, eighthNote},
		{D, eighthNote},
		{B, eighthNote},
		{A2, quarterNote},
			
		// 8
		{FS, quarterNote},
		{D, eighthNote},
		{CS, eighthNote},
		{D, eighthNote},
		{CS, eighthNote},
		{A2, quarterNote},
			
		// 9
		{FS, quarterNote},
		{A2, eighthNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{A2, eighthNote},
		{B2, dottedQuarter},
			
		// 10
		{CS2, eighthNote},
		{B2, dottedQuarter},
		{CS2, eighthNote},
		{B2, halfNote},

		// 11
		{A2, eighthNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{A2, eighthNote},
		{B2, dottedQuarter},
									
		// 12
		{CS2, eighthNote},
		{B2, dottedQuarter},
		{A2, eighthNote},
		{FS, quarterNote},
			
		// 13
		{0, quarterNote}, // rest
		{B2, eighthNote},
		{B2, eighthNote},
		{D2, eighthNote},
		{B2, eighthNote},
		{G2, quarterNote},
			
		// 14
		{FS2, quarterNote},
		{B2, eighthNote},
		{B2, eighthNote},
		{D2, eighthNote},
		{B2, eighthNote},
		{G2, quarterNote},

		// 15
		{FS2, quarterNote},
		{B2, eighthNote},
		{B2, eighthNote},
		{D2, eighthNote},
		{B2, eighthNote},
		{A3, quarterNote},
			
		// 16
		{FS2, quarterNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{A3, quarterNote},
			
		// 17 /14
		{FS2, quarterNote},
		{B2, eighthNote},
		{B2, eighthNote},
		{D2, eighthNote},
		{B2, eighthNote},
		{G2, quarterNote},
			
		// 18 /14
		{FS2, quarterNote},
		{B2, eighthNote},
		{B2, eighthNote},
		{D2, eighthNote},
		{B2, eighthNote},
		{G2, quarterNote},
			
		// 19 /15
		{FS2, quarterNote},
		{B2, eighthNote},
		{B2, eighthNote},
		{D2, eighthNote},
		{B2, eighthNote},
		{A3, quarterNote},
		
		// 20 /16
		{FS2, quarterNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{A3, quarterNote},

		// 21
		{FS2, quarterNote},
		{A2, eighthNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{B2, quarterNote},
			
		// 22 - pre-chorus
		{B2, quarterNote},
		{B2, quarterNote},
		{B2, quarterNote},
		{B2, eighthNote},
		{CS2, eighthNote},
			
		// 23
		{D2, quarterNote},
		{D2, quarterNote},
		{D2, quarterNote},
		{D2, eighthNote},
		{E2, eighthNote},
			
		// 24
		{FS2, quarterNote},
		{FS2, quarterNote},
		{FS2, quarterNote},
		{FS2, quarterNote},
			
		// 25
		{FS2, eighthNote},
		{CS2, eighthNote},
		{CS2, quarterNote},
		{FS2, eighthNote},
		{CS2, eighthNote},
		{CS2, eighthNote},
		{FS2, eighthNote},
			
		//26
		{B2, quarterNote},
		{B2, quarterNote},
		{B2, quarterNote},
		{B2, eighthNote},
		{CS2, eighthNote},
		
		// 27
		{D2, quarterNote},
		{D2, quarterNote},
		{D2, quarterNote},
		{D2, eighthNote},
		{E2, eighthNote},
		
		// 28
		{FS2, quarterNote},
		{FS2, quarterNote},
		{FS2, quarterNote},
		{FS2, quarterNote},
		
		// 29
		{FS2, eighthNote},
		{CS2, eighthNote},
		{CS2, eighthNote},
		{A2, eighthNote},
		{D2, eighthNote},
		{CS2, eighthNote},
		{B2SLIDE, eighthNote},
			
		// todo: chorus, maybe more precision
		{B, quarterNote},
		{B, quarterNote},
		{B, quarterNote},
		{B, eighthNote},
		{CS, eighthNote},
		
		// 23
		{D, quarterNote},
		{D, quarterNote},
		{D, quarterNote},
		{D, eighthNote},
		{E, eighthNote},
		
		// 24
		{FS, quarterNote},
		{FS, quarterNote},
		{FS, quarterNote},
		{FS, quarterNote},
		
		// 25
		{FS, eighthNote},
		{CS, eighthNote},
		{CS, quarterNote},
		{FS, eighthNote},
		{CS, eighthNote},
		{CS, eighthNote},
		{FS, eighthNote},
		
		//26
		{B, quarterNote},
		{B, quarterNote},
		{B, quarterNote},
		{B, eighthNote},
		{CS, eighthNote},
		
		// 27
		{D, quarterNote},
		{D, quarterNote},
		{D, quarterNote},
		{D, eighthNote},
		{E, eighthNote},
		
		// 28
		{FS, quarterNote},
		{FS, quarterNote},
		{FS, quarterNote},
		{FS, quarterNote},
		
		// 29
		{FS, eighthNote},
		{CS, eighthNote},
		{CS, quarterNote},

	};

	for(;;) {
		pos_lcd(0,0);
		puts_lcd2("1. Canon In D");
		pos_lcd(1,0);
		puts_lcd2("2. Beautiful Now");
		
		user_input = get_actual_key();
		if (user_input != 200) {
			{
				if (user_input == 1) {

					clr_lcd(); // dont remember
					pos_lcd(0,0);
					puts_lcd2("Now Playing:");
					pos_lcd(1,0);
					puts_lcd2("Canon In D");

					tempo = 160;
					play_music(canon_in_d, 115);
					clr_lcd();
				}
				if (user_input == 2) {

					clr_lcd(); // dont remember
					pos_lcd(0,0);
					puts_lcd2("Now Playing:");
					pos_lcd(1,0);
					puts_lcd2("Beautiful Now");

					tempo = 165;
					pos_lcd(0,0);
					puts_lcd2(sizeof(beautiful_now));
					
					play_music(beautiful_now, 161);
					clr_lcd();
				}
			}
		}
	}

	return 0;
}
