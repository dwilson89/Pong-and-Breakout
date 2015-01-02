/*
 * ADC.h
 *
 * Created: 20/12/2012 11:59:59 PM
 *  Author: Michael a.k.a The Baus
 */ 

#ifndef ADC_H_
#define ADC_H_

void ADCInitialise(unsigned char mux, unsigned char interrupt);
short ADCRead(void);

// Mux values for the ADC input pins
#define ADC0	0b000000
#define ADC1	0b000001
#define ADC4	0b000100
#define ADC5	0b000101
#define ADC6	0b000110
#define ADC7	0b000111
#define ADC8	0b100000
#define ADC9	0b100001
#define ADC10	0b100010
#define ADC11	0b100011
#define ADC12	0b100100
#define ADC13	0b100101

// Comparison sources for the ADC
#define ADC_REF_POWER     (1<<REFS0)
#define ADC_REF_INTERNAL  ((1<<REFS1) | (1<<REFS0))
#define ADC_REF_EXTERNAL  (0)

// These prescaler values are for high speed mode, ADHSM = 1
#if F_CPU == 16000000L
#define ADC_PRESCALER ((1<<ADPS2) | (1<<ADPS1))
#elif F_CPU == 8000000L
#define ADC_PRESCALER ((1<<ADPS2) | (1<<ADPS0))
#elif F_CPU == 4000000L
#define ADC_PRESCALER ((1<<ADPS2))
#elif F_CPU == 2000000L
#define ADC_PRESCALER ((1<<ADPS1) | (1<<ADPS0))
#elif F_CPU == 1000000L
#define ADC_PRESCALER ((1<<ADPS1))
#else
#define ADC_PRESCALER ((1<<ADPS0))
#endif

#endif /* ADC_H_ */