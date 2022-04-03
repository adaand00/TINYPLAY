/*
 * TinyPLay.c
 *
 * Created: 2021-08-28 18:41:26
 * Author : Adam
 */ 

#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#define DCLK 0
#define DDATA 2
#define BIN 1

unsigned char currentRow;
unsigned int numUpdates;

unsigned char rows[] = {0, 0b00010000, 0} ; 
unsigned char pressedright = 0;
unsigned char pressedleft = 0;
unsigned char playerPosition = 1;
unsigned char obsRow = 5;
unsigned char obsCol = 1;
unsigned char moved = 0;
unsigned char timeSincePress = 0;
unsigned char moveTime = 200;
unsigned char lastadd = 0;
volatile unsigned char endrows[] = {0b00010100, 0b00001000, 0b00010100};
unsigned char failed = 0;

int main( void )
{
	// set directions
	DDRB = 1<<DDATA | 1<<DCLK;
	// Disable digital input for button pin
	DIDR0 = 1<<BIN;
	// Set ADC pin	
	ADMUX = BIN;
	// Enable ADC and start conversion
	ADCSRA = 1<<ADEN | 1<<ADSC | 1<<ADATE | 0b100;
	

	// loop
	while (1)
	{
		_delay_ms(5);
		
		timeSincePress++;
		
		// Read analog input
		char butt = ADCL;
		char newpressedright;
		char newpressedleft;
		if(butt > 160){
			//both buttons pressed
			newpressedright = 1;	
			newpressedleft = 1;
		}else if(butt > 120){
			//right button pressed 				
			newpressedleft = 1;
			newpressedright = 0;	
		}else if(butt > 100){
			//left button pressed	
			newpressedleft = 0;
			newpressedright = 1;
		}else{
			//no button pressed
			newpressedright = 0;
			newpressedleft = 0;
		}
		
		//detect rising edge on left button
		if(newpressedleft & ~(pressedleft) & (playerPosition!=2)){
			playerPosition += 1;
			timeSincePress = 0;
		}
		
		//detect rising edge on right button
		if(newpressedright & ~(pressedright) & (playerPosition!=0)){
			playerPosition -= 1;
			timeSincePress = 0;
		}
		
		pressedright = newpressedright;
		pressedleft = newpressedleft;
		
		// Move obstacle down
		if(numUpdates++ > moveTime){
			
			rows[currentRow] = rows[currentRow] >> 1;
			moved++;
			
			
			
			// all rows have moved
			if(moved == 3){
				
				// detect obstacle
				if((rows[playerPosition] & 1))
				{
					failed = 1;
				}
				
				//reset counters
				moved = 0;
				numUpdates = 0;
				
				// Add new obstacle every n rows
				lastadd++;
				if(lastadd == 3){
					unsigned char row = timeSincePress & 0b11;
					if(row == 3){
						row = playerPosition;
					}
					rows[row] = rows[row] | 0b00100000;
					lastadd = 0;
					moveTime -= moveTime>>4;
				}
			}
		}
		
		
		// Draw column
		
		// Buffer layout:
		//  b7 < b6 < b5 < b4 < b3 < b2 < b1 < b0 
		//  R4 | R3 | R2 | R1 | R0 | C2 | C1 | C0
		//
		// Row source, column sink
		
		char buff = (rows[currentRow] | (playerPosition==currentRow))<<3 |((1<<currentRow)^0b00000111);
		buff = buff^0xFF;
		
		if (failed)
		{
			buff = 0b00000111;
		}
		
		// Serial shift out, highest bit first. 
		for(int i = 0; i < 8; i++)
		{
			// Set clock low
			PORTB &= ~(1<<DCLK);
			
			
			if(buff & 0x80){
				PORTB |= 1<<DDATA;
			}else{
				PORTB &= ~(1<<DDATA);
			}
			
			// Set clock High
			PORTB |= 1<<DCLK;

			buff = buff<<1;
		}
		
		// next row of multiplex
		if(++currentRow == 3)
		{
			currentRow = 0;
		}	
	}
}


