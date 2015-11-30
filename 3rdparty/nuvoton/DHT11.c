#include <stdio.h>
#include "Nano1X2Series.h"
#include "clk.h"

#define DHT11_D PD11
extern volatile uint16_t time_DHT11[40];
extern volatile uint32_t capture_count ;

void Init_DHT11(void)
{
		TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1000000);
		TIMER_SET_PRESCALE_VALUE(TIMER0, 11);
	    // Set compare value as large as possible, so don't need to worry about counter overrun too frequently.
    TIMER_SET_CMP_VALUE(TIMER0, 0xFFFFFF);
		TIMER_EnableCapture(TIMER0, TIMER_CAPTURE_TRIGGER_COUNTING_MODE, TIMER_CAPTURE_RISING_THEN_FALLING_EDGE);
		GPIO_SetMode(PD,BIT11,GPIO_PMD_OUTPUT);
		GPIO_ENABLE_PULL_UP(PD,BIT11);
}



void Read_DHT11(uint16_t DHT11[])
{
	
		uint32_t uu;
		uint16_t DHT11_Humidity,DHT11_temp;
	


		capture_count = 0;
    DHT11_Humidity = 0; 
		DHT11_temp = 0 ;
	
		PD11 = 0 ;
		CLK_SysTickDelay (18000); //18ms
		PD11 = 1;
		SYS_UnlockReg();
		SYS->PD_H_MFP  = (SYS->PD_H_MFP & ~SYS_PD_H_MFP_PD11_MFP_Msk) | SYS_PD_H_MFP_PD11_MFP_TMR0_CAP;
		   
		TIMER_EnableCaptureInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
		// Start Timer 0
    TIMER_Start(TIMER0);
		CLK_SysTickDelay(500000);
							
		TIMER_DisableCaptureInt(TIMER0);
		TIMER_Stop(TIMER0);
		
		if (capture_count >=41) {
		
			for (uu=1;uu<17;uu++)
			{
				if (time_DHT11[uu] >30 )
					DHT11_Humidity = ((DHT11_Humidity << 1) | 1);
				else
					DHT11_Humidity = DHT11_Humidity << 1 ;
			}	
			for (uu=17;uu<33;uu++)
			{
				if (time_DHT11[uu] >30 )
					DHT11_temp = ((DHT11_temp << 1)  | 1);
				else
					DHT11_temp = DHT11_temp << 1 ;
			}	
			DHT11[0] = DHT11_Humidity;
			DHT11[1] = DHT11_temp;
		}
		else
		{
			DHT11[0] = 0;
			DHT11[1] = 0;	
		}
		SYS->PD_H_MFP  = (SYS->PD_H_MFP & ~SYS_PD_H_MFP_PD11_MFP_Msk) | SYS_PD_H_MFP_PD11_MFP_GPD11 ;
		PD11 = 1;

}
