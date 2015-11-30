//
// LM75A Driver: Temperature sensor
//
// pin1: SDA : I2C Serial Data
// pin2: SCL : I2C Serail Clock
// pin3: OS  : OverTemp Shutdown output - opendrain
// pin4: GND 
// pin5: A2  : user-defined address bit 2
// pin6: A1  : user-defined address bit 1
// pin7: A0  : user-defined address bit 0
// pin8: VCC : 2.8V to 5.5V

#include <stdio.h>
#include <stdint.h>
#include <Nano1X2Series.h>
#include "sys.h"
#include "i2c.h"
#include "LM75A.h"

void LM75A_I2C_SingleWrite(uint8_t index, uint8_t data)
{
        I2C_START(LM75A_I2C_PORT);                         //Start
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag	
	
	      I2C_SET_DATA(LM75A_I2C_PORT, LM75A_I2C_SLA);         //send slave address
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		

	      I2C_SET_DATA(LM75A_I2C_PORT, index);               //send index
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		

	      I2C_SET_DATA(LM75A_I2C_PORT, data);                //send Data
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
				
				I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI|I2C_STO);//Stop
}

uint8_t LM75A_I2C_SingleRead(uint8_t index)
{
	uint8_t tmp;
	      I2C_START(LM75A_I2C_PORT);                         //Start
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
	
	      I2C_SET_DATA(LM75A_I2C_PORT, LM75A_I2C_SLA);         //send slave address+W
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		

	      I2C_SET_DATA(LM75A_I2C_PORT, index);               //send index
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
	
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_STA | I2C_SI);	//Start
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag	

		    I2C_SET_DATA(LM75A_I2C_PORT, (LM75A_I2C_SLA+1));     //send slave address+R
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag								
	
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag								
				tmp = I2C_GET_DATA(LM75A_I2C_PORT);                //read data   
	
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI|I2C_STO);//Stop
				return tmp;
}

uint16_t LM75A_I2C_DoubleRead(uint8_t index)
{
	uint8_t msb, lsb;
	uint16_t tmp;
	      I2C_START(LM75A_I2C_PORT);                         //Start
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
	
	      I2C_SET_DATA(LM75A_I2C_PORT, LM75A_I2C_SLA);      //send slave address+W
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		

	      I2C_SET_DATA(LM75A_I2C_PORT, index);               //send index
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
	
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_STA | I2C_SI);	//Start
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag	

		    I2C_SET_DATA(LM75A_I2C_PORT, (LM75A_I2C_SLA+1));     //send slave address+R
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag								
	
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag								
				msb = I2C_GET_DATA(LM75A_I2C_PORT);                //read data   

	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(LM75A_I2C_PORT);
        LM75A_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag								
				lsb = I2C_GET_DATA(LM75A_I2C_PORT);                //read data  				
	      I2C_SET_CONTROL_REG(LM75A_I2C_PORT, I2C_SI|I2C_STO);//Stop
				
				tmp = msb;
				tmp = tmp<<8 | lsb;
				return tmp;
}

uint8_t Read_LM75A_Config(void)
{
		return (LM75A_I2C_SingleRead(LM75A_CONF));
}

uint16_t Read_LM75A_Temp(void)
{
		return (LM75A_I2C_DoubleRead(LM75A_TEMP));
}
