//
// LM75A : Temperature Sensor
// 
#define LM75A_I2C_SLA          0x90
#define LM75A_I2C_PORT         I2C1
// LM75A Internal Registers 
#define LM75A_TEMP             0x00 // read only
#define LM75A_CONF             0x01 // R/W
#define LM75A_THYST            0x02 // R/W
#define LM75A_TOS              0x03 // R/W

extern uint8_t Read_LM75A_Config(void);

extern uint16_t Read_LM75A_Temp(void);
