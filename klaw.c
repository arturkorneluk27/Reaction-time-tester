#include "klaw.h"
#include "frdm_bsp.h"

volatile uint8_t S1_press=0;
volatile uint8_t S2_press=0;	// "1" - klawisz został wciśnięty "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;

void Klaw_Init(void)    //inicjalizacja
{
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;		// Włączenie portu A (zegar)
	PORTA->PCR[S1] |= PORT_PCR_MUX(1);
	PORTA->PCR[S2] |= PORT_PCR_MUX(1);
	PORTA->PCR[S3] |= PORT_PCR_MUX(1);
	PORTA->PCR[S4] |= PORT_PCR_MUX(1);
	PORTA->PCR[S1] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;    //pull-upy
	PORTA->PCR[S2] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[S3] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[S4] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}
void Klaw_S1_4_Int(void)      //przerwania    
{
	PORTA -> PCR[S1] |= PORT_PCR_IRQC(0xa);
	PORTA -> PCR[S2] |= PORT_PCR_IRQC(0xa);		//0x8 - poziom "0"; 0x9 - zbocze narastające; 0xa - zbocze opadające; 0xb - obydwa zbocza
	PORTA -> PCR[S3] |= PORT_PCR_IRQC(0xa);		
	PORTA -> PCR[S4] |= PORT_PCR_IRQC(0xa);
	NVIC_SetPriority(PORTA_IRQn, 3); 
	NVIC_ClearPendingIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTA_IRQn);
}

void PORTA_IRQHandler(void)	// Podprogram obsługi przerwania od klawiszy S1, S2, S3 i S4
{
	uint32_t buf;
	buf=PORTA->ISFR & (S1_MASK | S2_MASK | S3_MASK | S4_MASK);

	switch(buf)
	{
		case S1_MASK:	DELAY(100)	;	// Minimalizacja drgań zestyków
									if(!(PTA->PDIR&S1_MASK))		
									{
										if(!S1_press)
										{
											S1_press=1;
										}
									}									
									break;
		case S2_MASK:	DELAY(100)	;	// Minimalizacja drgań zestyków
									if(!(PTA->PDIR&S2_MASK))		
									{
										if(!S2_press)
										{
											S2_press=1;
										}
									}									
									break;
		case S3_MASK:	DELAY(100)	;	// Minimalizacja drgań zestyków
									if(!(PTA->PDIR&S3_MASK))		
									{
										if(!S3_press)
										{
											S3_press=1;
										}
									}
									break;
		case S4_MASK:	DELAY(100);		// Minimalizacja drgań zestyków
									if(!(PTA->PDIR&S4_MASK))		
									{
										if(!S4_press)
										{
											S4_press=1;
										}
									}
									break;
		default:			break;
	}	
	PORTA->ISFR |=  S1_MASK | S2_MASK | S3_MASK | S4_MASK;	// Kasowanie wszystkich bitów ISF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
}