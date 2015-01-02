/*
 * ADC.c
 *
 * Created: 20/12/2012 11:59:59 PM
 *  Author: Michael a.k.a Master of his domain
 */ 

#include "ADC.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static unsigned char _interrupt = 0; // We will use this value to remember whether we are using interrupts or not
static volatile unsigned char low, high; // This is our storage for the ADC conversions. We need to declare it as a volatile type
										 // since it may be modified within an interrupt routine (ISR). Volatile just means that
										 // the value might change 'by itself' and tells the compiler not to make certain assumptions.

// Initialise the ADC registers
void ADCInitialise(unsigned char mux, unsigned char interrupt) {
	_interrupt = interrupt; // Store whether we are using interrupts or not 
							// so we can read the results of either from the ADCRead function
	
    ADCSRB = (1 << ADHSM) | (mux & 0x20); // Configure high speed mode conversions
    ADMUX = ADC_REF_POWER | (mux & 0x1F); // Configure the input ADC pin we want
	
	ADCSRA = (1 << ADEN) | ADC_PRESCALER; // Enable the ADC and configure the prescaler (conversion speed)
	
	if(_interrupt) {
		//not too sure about this
		ADCSRA ^= (1<<ADIE) | (1<<ADSC); // Enable the ADC interrupts and begin conversions
		sei(); // Enable global interrupts, otherwise our ISR will never be called
	}
}

// Read the ADC conversion value. This function returns a short since our ADC is 10 bit (0-1023) 
// which doesn't fit in an 8 bit character.
short ADCRead(void)
{
	if(!_interrupt) {
		ADCSRA ^= (1<<ADSC); // Start the conversion
		
		//could possible use ADIF when not equal to it
		while (ADCSRA & (1<<ADSC)); // Waste time until conversion is finished
	
		// Read the low byte
		low = ADCL;
		// Read the high byte
		high = ADCH;
	}		
	
    return (high << 8) | low;
}

// Our interrupt routine, this is automatically called whenever the ADC interrupt is triggered
ISR(ADC_vect) {
	// Read the low byte
	low = ADCL;
	// Read the high byte
	high = ADCH;
	ADCSRA |= (1<<ADSC); 
}