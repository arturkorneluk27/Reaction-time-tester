#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include "klaw.h"
#include "DAC.h"
#include "ADC.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define DIV_CORE	8192	// Przerwanie co 0.195ms (~0.2ms)       
#define MASKA_10BIT	0x03FF

volatile uint16_t dac;
volatile uint16_t faza = 0;
volatile uint16_t mod = 128;  
volatile int16_t Sinus[1024];
volatile uint8_t dds_on = 0;
volatile uint32_t time_ms = 0;   // licznik ms
volatile uint8_t measure = 0;   // pomiar

uint8_t wynik_gracz1 = 0;
uint8_t wynik_gracz2 = 0;


void Delay_ms(uint32_t ms)
{
	time_ms = 0;
	measure = 1;  // pomiar czasu start
	while(time_ms < ms);  
	measure = 0;  // pomiar czasu stop
}


uint32_t ADC_random_seed(void)    // losowosc z szumu na nieuzywanym kanale ADC
{
	uint32_t seed = 0;
	uint16_t adc_val;
	uint8_t i;
	
	ADC0->SC3 = 0;  // brak uśredniania
	
	// szum
	for(i = 0; i < 32; i++)
	{
		ADC0->SC1[0] = ADC_SC1_ADCH(9);  // kanał 9 
		while(!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));  
		adc_val = ADC0->R[0];  // śmieci z nieużywanego kanału
		seed = (seed << 1) | (adc_val & 0x01);		// najmlodszy bit
	}
	
	seed ^= SysTick->VAL;
	ADC0->SC3 = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3); // przywrocenie usredniania

	return seed;
}



void SysTick_Handler(void)	// Podprogram obsługi przerwania od SysTick'a
{ 
	static uint16_t tick = 0;
	
	if(dds_on)      // generacja dzwieku
	{
		dac = Sinus[faza] + 2048;		
		DAC_Load_Trig(dac);
		faza+=mod;		
		faza&=MASKA_10BIT;	
	}
	else
	{
		DAC_Load_Trig(2048);   // srodek zakresu - cisza
	}
	
	tick++;
   if(tick >= 5)         // 5*0.195ms = 0.975ms (~1ms)
	{
      tick = 0;
			if(measure)
				time_ms++;
	}
}



int main(void)
{
	S1_press = 0;  // zerowanie flag przyciskow
	S2_press = 0;
	S4_press = 0;
 
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	uint8_t round;
	uint32_t random_seed;
	uint32_t wait;
	uint8_t	kal_error;
	
  Klaw_Init();
  __enable_irq(); 
  Klaw_S1_4_Int();

    
  LCD1602_Init();
  LCD1602_Backlight(TRUE);
	DAC_Init();
		
	kal_error = ADC_Init();
	if(kal_error)
	{
		while(1);  // Kalibracja się nie powiodła
	}
		
	random_seed = ADC_random_seed();
	srand(random_seed);  
	
	for(faza=0;faza<1024;faza++)
		Sinus[faza]=(sin((double)faza*0.0061359231515)*2047.0);   // Ładowanie 1024-ech sztuk, 12-bitowych próbek funkcji sinus do tablicy
		
	NVIC_SetPriority(SysTick_IRQn, 0);
	SysTick_Config(SystemCoreClock/DIV_CORE);       // Start licznika SysTick 

	dds_on = 0;   // wartosci poczatkowe
	measure = 0;
	time_ms = 0;
	DAC_Load_Trig(2048);
		
	while(1)
	{
    LCD1602_ClearAll();
    LCD1602_Print("REFLEKSOMIERZ");
    LCD1602_SetCursor(0,1);
    LCD1602_Print("S1 = START");

		while(!S1_press);     // start
		S1_press = 0;
		
		wynik_gracz1 = 0;
		wynik_gracz2 = 0;

		for(round =1; round <=3; round++)   // 3 rundy
		{
			LCD1602_ClearAll();
			sprintf(display, "Runda %d", round);
			LCD1602_Print(display);
			LCD1602_SetCursor(0,1);
			LCD1602_Print("Czekaj...");
			
			wait = (rand() % 6000) + 4000;   //  4–10s losowy czas

			uint8_t false_start = 0;
			
			time_ms = 0;
			measure = 1;  // pomiar czasu
			S2_press = 0;
			S4_press = 0;

			while(time_ms < wait)   // falstart
			{         
          		if(S2_press || S4_press)
          		{
									false_start = 1;
              		LCD1602_ClearAll();
              		LCD1602_Print("FALSTART!");
              		LCD1602_SetCursor(0,1);
                    
              	if(S2_press && S4_press)
              	{
									LCD1602_Print("REMIS - brak kar");
              	}
              	else if(S2_press)
              	{
									LCD1602_Print("Gracz 1 - kara");
                  wynik_gracz2++;  // punkt dla przeciwnika
              	}
              	else
              	{
										LCD1602_Print("Gracz 2 - kara");
                  	wynik_gracz1++;  // punkt dla przeciwnika
              	}
              	break;
          		}
      		}
            
			if(false_start == 1) // wystapil falstart
      {
				measure = 0;  // stop czas
				Delay_ms(6000);  // 6000*0.975ms = 5.85s (~6s)
				continue;  
      }
			
			S2_press = 0;  // reset przed sygnalem start
			S4_press = 0;
			
			faza = 0;
      mod = 128;
      dds_on = 1;
            
      time_ms = 0;
      measure = 1;
            
			while(time_ms < 100);  // sygnal startowy ~100ms
			
			dds_on = 0;
			measure = 0;
			
			time_ms = 0;
			measure = 1;
			
      LCD1602_ClearAll();
      LCD1602_Print("REAGUJ!");
            
      S2_press = 0;
      S4_press = 0;
            
    while(time_ms < 4000)    // pomiar
    {
			if(S2_press)
			{
				dds_on = 0;
				measure = 0;
				wynik_gracz1++;    // punkt
				sprintf(display, "Gracz 1: %u ms", time_ms);
				LCD1602_ClearAll();
				LCD1602_Print(display);
				Delay_ms(3000);  // 3000*0.975ms = 2.925s (~3s)
				S2_press = 0;
				break;
			}
			if(S4_press)
			{
				dds_on = 0;
				measure = 0;
				wynik_gracz2++;   // punkt
				sprintf(display, "Gracz 2: %u ms", time_ms);
				LCD1602_ClearAll();
				LCD1602_Print(display);
				Delay_ms(3000);  // ~3s     
				S4_press = 0;
				break;
			}
    }
			if(time_ms >= 4000)   // brak reakcji
			{
				dds_on = 0;
				measure = 0;
				LCD1602_ClearAll();
				LCD1602_Print("BRAK REAKCJI");
				Delay_ms(3000);  // ~3s;
			}
		}
		
		LCD1602_ClearAll();
		sprintf(display, "G1:%d        G2:%d", wynik_gracz1, wynik_gracz2);   // wyniki
		LCD1602_Print(display);
		LCD1602_SetCursor(0,1);
		
		if(wynik_gracz1 > wynik_gracz2)
			LCD1602_Print("Gracz 1 WYGRAL!");
		
		else if(wynik_gracz1 < wynik_gracz2)
			LCD1602_Print("Gracz 2 WYGRAL!");
		
		else
			LCD1602_Print("REMIS!");
		
		Delay_ms(8000);  // 8000*0.975ms= 7.8s (~8s)
	
	S1_press = 0;  // resetujemy flagi na koniec by podczas wyswietlania wynikow niezapisac w pamieci niechcianych ruchow
	S2_press = 0;
	S4_press = 0;
	}
}

