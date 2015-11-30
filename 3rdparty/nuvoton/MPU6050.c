//
// MPU6050 Driver: 3-axis Gyroscope + 3-axis accelerometer + temperature
//
// Interface: I2C
// pin1: Vcc to Vcc (+5V)
// pin2: Gnd to Gnd
// pin3: SCL to I2C1_SCL/PC10
// pin4: SDA to I2C1_SDA/PC11
// pin5: XDA -- N.C.
// pin6: XCL -- N.C.
// pin7: AD0 -- N.C.
// pin8: INT -- N.C.

#include <stdio.h>
#include <stdint.h>
#include <Nano1X2Series.h>
#include "sys.h"
#include "gpio.h"
#include "i2c.h"
#include "MPU6050.h"

void MPU6050_I2C_SingleWrite(uint8_t index, uint8_t data)
{
        I2C_START(MPU6050_I2C_PORT);                         //Start
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag	
	
	      I2C_SET_DATA(MPU6050_I2C_PORT, MPU6050_I2C_SLA);         //send slave address
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		

	      I2C_SET_DATA(MPU6050_I2C_PORT, index);               //send index
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		

	      I2C_SET_DATA(MPU6050_I2C_PORT, data);                //send Data
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
				
				I2C_STOP(MPU6050_I2C_PORT); //STOP
}

uint8_t MPU6050_I2C_SingleRead(uint8_t index)
{
	uint8_t tmp;
	      I2C_START(MPU6050_I2C_PORT);                         //Start
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
	
	      I2C_SET_DATA(MPU6050_I2C_PORT, MPU6050_I2C_SLA);         //send slave address+W
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		

	      I2C_SET_DATA(MPU6050_I2C_PORT, index);               //send index
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag		
	
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_STA | I2C_SI);	//Start
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag	

		    I2C_SET_DATA(MPU6050_I2C_PORT, (MPU6050_I2C_SLA+1));     //send slave address+R
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag								
	
	      I2C_SET_CONTROL_REG(MPU6050_I2C_PORT, I2C_SI);
	      I2C_WAIT_READY(MPU6050_I2C_PORT);
        MPU6050_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag								
				tmp = I2C_GET_DATA(MPU6050_I2C_PORT);                //read data   
	
	      I2C_STOP(MPU6050_I2C_PORT); //STOP
				return tmp;
}

void Init_MPU6050(void)
{
  I2C_SetSlaveAddr(MPU6050_I2C_PORT, 0, MPU6050_I2C_SLA, I2C_GCMODE_DISABLE);	
	MPU6050_I2C_SingleWrite(MPU6050_PWR_MGMT_1, 0x01);	// CLL_SEL=0: internal 8MHz, TEMP_DIS=0, SLEEP=0 
	MPU6050_I2C_SingleWrite(MPU6050_SMPLRT_DIV, 0x01);  // Gyro output sample rate = Gyro Output Rate/(1+SMPLRT_DIV)
	MPU6050_I2C_SingleWrite(MPU6050_CONFIG, 0x01);      // set TEMP_OUT_L, DLPF=2 (Fs=1KHz)
	MPU6050_I2C_SingleWrite(MPU6050_GYRO_CONFIG, 0x18); // bit[4:3] 0=+-250d/s,1=+-500d/s,2=+-1000d/s,3=+-2000d/s
	MPU6050_I2C_SingleWrite(MPU6050_ACCEL_CONFIG, 0x00);// bit[4:3] 0=+-2g,1=+-4g,2=+-8g,3=+-16g, ACC_HPF=On (5Hz)
}

uint16_t Read_MPU6050_AccX(void)
{
	uint8_t LoByte, HiByte;
	LoByte = MPU6050_I2C_SingleRead(MPU6050_ACCEL_XOUT_L); // read Accelerometer X_Low  value
	HiByte = MPU6050_I2C_SingleRead(MPU6050_ACCEL_XOUT_H); // read Accelerometer X_High value
	return((HiByte<<8) | LoByte);
}

uint16_t Read_MPU6050_AccY(void)
{
	uint8_t LoByte, HiByte;
	LoByte = MPU6050_I2C_SingleRead(MPU6050_ACCEL_YOUT_L); // read Accelerometer X_Low  value
	HiByte = MPU6050_I2C_SingleRead(MPU6050_ACCEL_YOUT_H); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

uint16_t Read_MPU6050_AccZ(void)
{
	uint8_t LoByte, HiByte;
	LoByte = MPU6050_I2C_SingleRead(MPU6050_ACCEL_ZOUT_L); // read Accelerometer X_Low  value
	HiByte = MPU6050_I2C_SingleRead(MPU6050_ACCEL_ZOUT_H); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

uint16_t Read_MPU6050_GyroX(void)
{
	uint8_t LoByte, HiByte;
	LoByte = MPU6050_I2C_SingleRead(MPU6050_GYRO_XOUT_L); // read Accelerometer X_Low  value
	HiByte = MPU6050_I2C_SingleRead(MPU6050_GYRO_XOUT_H); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

uint16_t Read_MPU6050_GyroY(void)
{
	uint8_t LoByte, HiByte;
	LoByte = MPU6050_I2C_SingleRead(MPU6050_GYRO_YOUT_L); // read Accelerometer X_Low  value
	HiByte = MPU6050_I2C_SingleRead(MPU6050_GYRO_YOUT_H); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

uint16_t Read_MPU6050_GyroZ(void)
{
	uint8_t LoByte, HiByte;
	LoByte = MPU6050_I2C_SingleRead(MPU6050_GYRO_ZOUT_L); // read Accelerometer X_Low  value
	HiByte = MPU6050_I2C_SingleRead(MPU6050_GYRO_ZOUT_H); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}
