//
// smpl_Nuwarocks : use all sensors of Nuwarocks v1.0
//
// MCU : Nano102SC2AN
// EVB : NUWAROCKS v1.0

// I2C1 to OLED, LM75A, MPU6050
// PC10/I2C1-SCL to I2C-Slave SCL
// PC11/I2C1-SDA to I2C-Slave SDA

// GPIOs
// PC4  to BT Reset
// PC6  to WiFi Reset
// PC12 to WiFi Sleep
// PC13 to MPU6050 INT
// PB12 to KEY1
// PB13 to KEY2
// PB14 to KEY3

// PD9 /PWM0_CH3 to Buzzer

// PD11 to DHT11-IN
// PD12 to H-EN (enable MQ7)

// PA4 /ADC4 to GL5516/IR diode
// PA5 /ADC5 to MQ7-Out

// UART0 to BT   module
// PA13/UART0-RX to BT-TX
// PA12/UART0-TX to BT-RX

// UART1 to WiFi module
// PC7 /UART1-RX to WiFi-TX
// PC8 /UART1-TX to WiFi-RX


#define BMP280_I2C_SLA          0xEE
#define BMP280_I2C_PORT         I2C1

#include <stdio.h>
#include <string.h>
#include "Nano1X2Series.h"
#include "LM75A.h"
#include "MPU6050.h"
#include "LY096BG30.h"
#include "BSP_Driver.h"
#include "DHT11.h"
#include "RAK415.h"
#include "systimer.h"

#include <unabto/unabto.h>
#include "unabto_application.h"

char nabtoId[] = "xxxxxxxxxxxx.nuvoton.sdk.u.nabto.net";
//char nabtoId[] = "xxxxxxxxxxxx.demo.nab.to";

volatile char     Text[51] = {'@','A','C','C',0,0,0,0,0,0, //offset 4
                                                            '@','G','Y','R',0,0,0,0,0,0, //offset 14
                                                            '@','T','E','M',0,0,  //offset 24
                                                            '@','H','R','W',0,0, //offset 30
                                                            '@','L','I','R',0,0, //offset 36
                                                            '@','G','A','S',0,0, //offset 42
                                                            '@','D','I','S',0,  //offset 48
                                                             0x0a,0x0d  }   ;
volatile char     Text_ACK[4];
volatile uint8_t  command[4];
volatile uint8_t  Flag_report =0;
volatile uint8_t  KEY1_Flag = 0;
volatile uint8_t  KEY2_Flag = 0;
volatile uint8_t  KEY3_Flag = 0, report_rate=1 ,Report_on =0;
volatile uint32_t g_u32TICK = 0,Timer_F;
volatile uint32_t LightValue, GasValue;
volatile uint16_t AcceX, AcceY,AcceZ,GyroX,GyroY,GyroZ,Humidity,Temp;

volatile uint16_t time_DHT11[40];
volatile uint32_t capture_count =0;
volatile uint16_t DHT11_Data[2];

void TMR0_IRQHandler(void)
{
    time_DHT11[capture_count] = TIMER_GetCaptureData(TIMER0);
        capture_count++;
    TIMER_ClearCaptureIntFlag(TIMER0);
}

void UART0_IRQHandler(void)
{
    uint8_t  RX_buffer[16];
    uint32_t u32IntSts= UART0->ISR;
    Flag_report =0;

    if(u32IntSts && UART_IS_RX_READY(UART0)) {
      UART_Read(UART0, RX_buffer, 8);
    }

        if ((RX_buffer[4]==0)&& (RX_buffer[6]==1))
        {
            Flag_report =3 ;
        }
        else
            Flag_report =0;
}

// FIXME
static volatile int n_wifi_data_loss;    // Track how many WiFi data are lost
void UART1_IRQHandler(void)
{
    if (UART1->ISR & UART_ISR_RDA_IS_Msk) {
        // Get all the input characters
#ifdef TIMER1_PUNC_SOCK
        TIMER_Stop(TIMER1);
        TIMER_ResetCounter(TIMER1);
#endif
        while (UART_IS_RX_READY(UART1)) {
            // Get the character from UART Buffer
            uint8_t bydata = UART_READ(UART1);

            if (! rakmgr_uartbuf_full()) {
                rakmgr_uartbuf_putc(bydata);
            }
            else {
                n_wifi_data_loss ++;
            }
        }
#ifdef TIMER1_PUNC_SOCK
        TIMER_Start(TIMER1);
#endif
    }
}

void GPABC_IRQHandler(void)
{
    if (PB->ISRC & BIT12) {        // check if PB12 interrupt occurred
        PB->ISRC |= BIT12;         // clear PB12 interrupt status
              KEY1_Flag=1;               // set a flag for PB12(KEY1)
    } else if (PB->ISRC & BIT13) { // check if PB13 interrupt occurred
        PB->ISRC |= BIT13;         // clear PB13 interrupt status
        KEY2_Flag=1;                  // set a flag for PB13(KEY2)
    } else if (PB->ISRC & BIT14) { // check if PB14 interrupt occurred
        PB->ISRC |= BIT14;         // clear PB14 interrupt status
        KEY3_Flag=1;                  // set a flag for PB14(KEY3)
    } else {                      // else it is unexpected interrupts
        PB->ISRC = PB->ISRC;          // clear all GPC pins
    }
}

void ADC_IRQHandler(void)
{
    uint32_t u32Flag;

    // Get ADC conversion finish interrupt flag
    u32Flag = ADC_GET_INT_FLAG(ADC, ADC_ADF_INT);

    if(u32Flag & ADC_ADF_INT) {
        LightValue = ADC_GET_CONVERSION_DATA(ADC, 4);
        GasValue = ADC_GET_CONVERSION_DATA(ADC, 5);
    }
    ADC_CLR_INT_FLAG(ADC, u32Flag);
}

void RTC_IRQHandler(void)
{
    S_RTC_TIME_DATA_T sCurTime;
    if ( (RTC->RIER & RTC_RIER_TIER_Msk) && (RTC->RIIR & RTC_RIIR_TIF_Msk) ) { // RTC interrupt
        RTC->RIIR = 0x2;

    RTC_GetDateAndTime(&sCurTime); // Get the current time

    g_u32TICK++;
    }
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable Internal RC (12 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC_EN_Msk | CLK_PWRCTL_LXT_EN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_HIRC_STB_Msk | CLK_CLKSTATUS_LXT_STB_Msk);

    /* Set HCLK clock source */
    //CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_HCLK_CLK_DIVIDER(1));
    CLK_SetCoreClock(32000000);

    /* Select IP clock source */
    CLK_SetModuleClock(I2C1_MODULE, 0, 0);
    CLK_SetModuleClock(RTC_MODULE, 0, 0);
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL1_ADC_S_HIRC, CLK_ADC_CLK_DIVIDER(1));
    CLK_SetModuleClock(PWM0_CH23_MODULE, CLK_CLKSEL1_PWM0_CH23_S_HIRC, 0);
    CLK_SetModuleClock(TMR0_MODULE,  CLK_CLKSEL1_TMR0_S_HIRC, CLK_TMR0_CLK_DIVIDER(1));
    //CLK_SetModuleClock(TMR0_MODULE,  CLK_CLKSEL1_TMR1_S_HIRC, CLK_TMR0_CLK_DIVIDER(3));   // 12/3=4MHz
    CLK_SetModuleClock(TMR1_MODULE,  CLK_CLKSEL1_TMR1_S_HIRC, CLK_TMR1_CLK_DIVIDER(3)); // 12/3=4MHz
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_UART_CLK_DIVIDER(3));
    CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_UART_CLK_DIVIDER(3));


    /* Enable IP clock */
    CLK_EnableModuleClock(I2C1_MODULE);
    CLK_EnableModuleClock(RTC_MODULE);
    CLK_EnableModuleClock(ADC_MODULE);
    CLK_EnableModuleClock(PWM0_CH23_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(TMR1_MODULE);
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(UART1_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set multi function pin for I2C1 */
        SYS->PC_H_MFP &= ~(SYS_PC_H_MFP_PC10_MFP_Msk);
    SYS->PC_H_MFP |= (SYS_PC_H_MFP_PC10_MFP_I2C1_SCL);
        SYS->PC_H_MFP &= ~(SYS_PC_H_MFP_PC11_MFP_Msk);
    SYS->PC_H_MFP |= (SYS_PC_H_MFP_PC11_MFP_I2C1_SDA);

    /* Set PA multi-function pins for ADC */
    SYS->PA_L_MFP &= ~SYS_PA_L_MFP_PA4_MFP_Msk;
    SYS->PA_L_MFP |= SYS_PA_L_MFP_PA4_MFP_ADC_CH4;
    SYS->PA_L_MFP &= ~SYS_PA_L_MFP_PA5_MFP_Msk;
    SYS->PA_L_MFP |= SYS_PA_L_MFP_PA5_MFP_ADC_CH5;

    /* Disable digital input path */
    PA->OFFD = PA->OFFD | (ADC_CH_4_MASK << 16);
    PA->OFFD = PA->OFFD | (ADC_CH_5_MASK << 16);

    /* Set PD multi-function pins for PWM */
    SYS->PD_H_MFP = (SYS->PD_H_MFP & ~(SYS_PD_H_MFP_PD9_MFP_Msk) )| SYS_PD_H_MFP_PD9_MFP_PWM0_CH3;

    /* Set multi function pin for UART0 */
      SYS->PA_H_MFP &=  ~(SYS_PA_H_MFP_PA12_MFP_Msk);
        SYS->PA_H_MFP |=   (SYS_PA_H_MFP_PA12_MFP_UART0_TX); // LQFP64-pin56
        SYS->PA_H_MFP &=  ~(SYS_PA_H_MFP_PA13_MFP_Msk);
        SYS->PA_H_MFP |=   (SYS_PA_H_MFP_PA13_MFP_UART0_RX); // LQFP64-pin57

    /* Set multi function pin for UART1 */
      SYS->PC_L_MFP &=  ~(SYS_PC_L_MFP_PC7_MFP_Msk);
        SYS->PC_L_MFP |=   (SYS_PC_L_MFP_PC7_MFP_UART1_RX); // LQFP64-pin14
        SYS->PC_H_MFP &=  ~(SYS_PC_H_MFP_PC8_MFP_Msk);
        SYS->PC_H_MFP |=   (SYS_PC_H_MFP_PC8_MFP_UART1_TX); // LQFP64-pin15

    /* Lock protected registers */
    SYS_LockReg();
}

void Init_RTC(void)
{
        S_RTC_TIME_DATA_T sInitTime;

    /* Time Setting */
    sInitTime.u32Year       = 2014;
    sInitTime.u32Month      = 7;
    sInitTime.u32Day        = 3;
    sInitTime.u32Hour       = 13;
    sInitTime.u32Minute     = 30;
    sInitTime.u32Second     = 0;
    sInitTime.u32DayOfWeek  = RTC_THURSDAY;
    sInitTime.u32TimeScale  = RTC_CLOCK_24;

      RTC_Open(&sInitTime);

    /* Set Tick Period */
    RTC_SetTickPeriod(RTC_TICK_1_SEC);

    /* Enable RTC Tick Interrupt */
    RTC_EnableInt(RTC_RIER_TIER_Msk);
    NVIC_EnableIRQ(RTC_IRQn);
}

void Init_ADC(void)
{
    ADC_Open(ADC,
        ADC_INPUT_MODE_SINGLE_END,
        /*ADC_OPERATION_MODE_SINGLE_CYCLE*/ADC_OPERATION_MODE_CONTINUOUS,
        ADC_CH_4_MASK | ADC_CH_5_MASK );

    // Power on ADC
    ADC_POWER_ON(ADC);

    // Use polling mode instead. Interrupt of high frequency will influence main thread of execution per test.
    //ADC_EnableInt(ADC, ADC_ADF_INT);
    //NVIC_EnableIRQ(ADC_IRQn);

    ADC_START_CONV(ADC);
}

void Init_GPIO(void)
{
      GPIO_SetMode(PC, BIT6,  GPIO_PMD_OUTPUT); // WiFi Reset
      PC6=1; // WiFi Reset
      GPIO_SetMode(PC, BIT12, GPIO_PMD_OUTPUT); // WiFi Sleep
      PC12=0;

    GPIO_SetMode(PA, BIT14,  GPIO_PMD_OUTPUT);  // LED (green)
    PA14 = 1;                                   // Off
    GPIO_SetMode(PB, BIT15,  GPIO_PMD_OUTPUT);  // LED (red)
    PB15 = 1;                                   // Off
    GPIO_SetMode(PD, BIT10, GPIO_PMD_OUTPUT);   // LED (blue)
    PD10 = 1;                                   // Off
}

void Init_KEY(void)
{
    GPIO_SetMode(PB, (BIT12 | BIT13 | BIT14), GPIO_PMD_INPUT);
    GPIO_ENABLE_PULL_UP(PB, (BIT12 | BIT13 | BIT14));
    GPIO_EnableInt(PB, 12, GPIO_INT_FALLING);
    GPIO_EnableInt(PB, 13, GPIO_INT_FALLING);
    GPIO_EnableInt(PB, 14, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPABC_IRQn);
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_HCLK, GPIO_DBCLKSEL_64);
    GPIO_ENABLE_DEBOUNCE(PB, (BIT12 | BIT13 | BIT14));
}

void Init_PWM(void)
{
    // PWM0 CH3 frequency is 1KHz, duty 50%,
    PWM_ConfigOutputChannel(PWM0, 3, 1000, 50);
    PWM_EnableOutput(PWM0, BIT3);
    PWM_Start(PWM0, BIT3);
    PWM_DisableOutput(PWM0, BIT3);
}

#ifdef TIMER1_PUNC_SOCK
void Init_Timer1(void)
{
    // FIXME
    //TIMER1->TCMPR = __XTAL*100/115200;      // For 1 tick per second when using external 12MHz (Prescale = 1)
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 115200 / 100);
    TIMER_Stop(TIMER1); // TIMER_Open() will trigger timer. Stop it immediately.
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);

}
#endif

void Init_Bluetooth(void)
{
      UART_Open(UART0,115200);  // enable UART0 at 115200 baudrate
    UART_ENABLE_INT(UART0, UART_IER_RDA_IE_Msk);
    NVIC_EnableIRQ(UART0_IRQn);
}
void Init_WiFi(void)
{
      UART_Open(UART1,115200);  // enable UART1 at 115200 baudrate
    UART_ENABLE_INT(UART1, UART_IER_RDA_IE_Msk);
    NVIC_EnableIRQ(UART1_IRQn);
}

void Buzz(uint8_t no)
{
    uint8_t i;
    for (i=0;i<no;i++) {
            PWM_EnableOutput(PWM0, BIT3);
            CLK_SysTickDelay(100000);
            PWM_DisableOutput(PWM0, BIT3);
            CLK_SysTickDelay(100000);
        }
}

#ifdef TIMER1_PUNC_SOCK
void TMR1_IRQHandler(void)
{
    TIMER_ClearIntFlag(TIMER1);
    TIMER_Stop(TIMER1);
    TIMER_ResetCounter(TIMER1);
    rakmgr_uart_data_recv = 1;
}
#endif


uint8_t BLE_Test()
{
    uint8_t BLE_CMD[11]={'i','t','c','z',0, 4, 0x54, 0x045, 0x53, 0x54, 0xc0};
    return UART_Write(UART0, BLE_CMD, 11);
}

void populateNabtoId()
{
    // Use modules mac address as unique Nabto ID
    int retval = rak_query_macaddr();
    if (retval < 0) {
        NABTO_LOG_DEBUG(("rak_query_macaddr failed=%u", retval));
        return;
    }

    rakmgr_sockdata_prg(-1);
    retval = rakmgr_sockdata_timeout(RAK_QMACTIMEOUT, rakmgr_sockdata_poll_at_resp);
    if (retval < 0) {
        NABTO_LOG_DEBUG(("rakmgr_sockdata_timeout failed=%u", retval));
        return;
    }

    if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2) == 0) {
        char macString[13];
        sprintf(macString, "%x%x%x%x%x%x", uCmdRspFrame.uCmdRspBuf[2], uCmdRspFrame.uCmdRspBuf[3], uCmdRspFrame.uCmdRspBuf[4], uCmdRspFrame.uCmdRspBuf[5], uCmdRspFrame.uCmdRspBuf[6], uCmdRspFrame.uCmdRspBuf[7]);
        strncpy(nabtoId, macString, 12);
    }
}

/**
  * Turn on/off LED (blue) as intensity of ambient light (per light sensor GL5516) transition is
  * high-to-low/low-to-high.
  */
void poll_ltsens_n_turn_led(void)
{
    static uint32_t LightValue_prev = 0;
    uint32_t u32Flag = ADC_GET_INT_FLAG(ADC, ADC_ADF_INT);

#define LT_ISI_THR  1000    // Light intensity threshold
    if (u32Flag & ADC_ADF_INT) {
        LightValue = ADC_GET_CONVERSION_DATA(ADC, 4);
        //GasValue = ADC_GET_CONVERSION_DATA(ADC, 5);

        if ((LightValue > LT_ISI_THR && LightValue_prev <= LT_ISI_THR) ||
            (LightValue <= LT_ISI_THR && LightValue_prev > LT_ISI_THR)) {
            PD10 = (LightValue > LT_ISI_THR) ? 0 : 1;
        }
        LightValue_prev = LightValue;
    }
    ADC_CLR_INT_FLAG(ADC, u32Flag);
}

void Nabto_Test()
{
    nabto_main_setup* nms;
    int i;

    rakmgr_reset_rak();
    delay_ms(200);

    if (rakmgr_open_cmdMode() < 0) {
        NABTO_LOG_ERROR(("WiFi: Open AT mode failed"));
        return;
    }

    // Check WiFi configuration. If the configuration doesn't fit nabto application, reconfigure it.
    if (rakmgr_read_userConfig() < 0) {
        NABTO_LOG_ERROR(("WiFi: Read user configuration failed"));
        return;
    }
    if (rak_userCfgstr.conn_multi != '1' ||
        strncmp(rak_userCfgstr.tcp_udp[0].protocol, "ludp", 4) ||
        strncmp(rak_userCfgstr.tcp_udp[0].local_port, "5570", 4) ||
        strncmp(rak_userCfgstr.tcp_udp[1].protocol, "ludp", 4) ||
        strncmp(rak_userCfgstr.tcp_udp[1].local_port, "25001", 5)) {
        if (rakmgr_wrt_user_conf() < 0) {
            NABTO_LOG_ERROR(("WiFi: Write user configuration failed"));
            return;
        }
        if (rakmgr_read_userConfig() < 0) {
            NABTO_LOG_ERROR(("WiFi: Read user configuration failed (2)"));
            return;
        }
    }

    platform_initialize();
    nms = unabto_init_context();

    populateNabtoId();

    nms->id = nabtoId;
    unabto_init();

#if NABTO_ENABLE_LOGGING == 0
    print_Line(0, "Nabto Demo:");
    for (i=0; i<12; i++) {
        print_C(i, 1, nabtoId[i]);
    }
    {
        int n = strlen(nabtoId) - 12;
        if (n > 16) {
            n = 16;
        }
        for (i=0; i<n; i++) {
            print_C(i, 2, nabtoId[i+12]);
        }
    }
#endif

    while(1) {
        unabto_tick();

        poll_ltsens_n_turn_led();

        if (n_wifi_data_loss) {
            NABTO_LOG_ERROR(("WiFi: %d bytes lost", n_wifi_data_loss))
            n_wifi_data_loss = 0;
        }
    }
}

uint8_t BMP280_I2C_SingleRead(uint8_t index)
{
    uint8_t tmp;
          I2C_START(BMP280_I2C_PORT);                         //Start
          I2C_WAIT_READY(BMP280_I2C_PORT);
        BMP280_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag

          I2C_SET_DATA(BMP280_I2C_PORT, BMP280_I2C_SLA);         //send slave address+W
          I2C_SET_CONTROL_REG(BMP280_I2C_PORT, I2C_SI);
          I2C_WAIT_READY(BMP280_I2C_PORT);
        BMP280_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag

          I2C_SET_DATA(BMP280_I2C_PORT, index);               //send index
          I2C_SET_CONTROL_REG(BMP280_I2C_PORT, I2C_SI);
          I2C_WAIT_READY(BMP280_I2C_PORT);
        BMP280_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag

          I2C_SET_CONTROL_REG(BMP280_I2C_PORT, I2C_STA | I2C_SI);   //Start
          I2C_WAIT_READY(BMP280_I2C_PORT);
        BMP280_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag

            I2C_SET_DATA(BMP280_I2C_PORT, (BMP280_I2C_SLA+1));     //send slave address+R
          I2C_SET_CONTROL_REG(BMP280_I2C_PORT, I2C_SI);
          I2C_WAIT_READY(BMP280_I2C_PORT);
        BMP280_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag

          I2C_SET_CONTROL_REG(BMP280_I2C_PORT, I2C_SI);
          I2C_WAIT_READY(BMP280_I2C_PORT);
        BMP280_I2C_PORT->INTSTS |= I2C_INTSTS_INTSTS_Msk;   //clear flag
                tmp = I2C_GET_DATA(BMP280_I2C_PORT);                //read data

          I2C_SET_CONTROL_REG(BMP280_I2C_PORT, I2C_SI|I2C_STO);//Stop
                return tmp;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Main Function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main()
{
    SYS_Init();

    systimer_init();
    Init_ADC();     // Light sensor GL5516
    Init_GPIO();    // WiFi Reset/Sleep pins.

#ifdef TIMER1_PUNC_SOCK
    Init_Timer1();
#endif
    Init_WiFi();

#if NABTO_ENABLE_LOGGING == 0
    I2C_Open(I2C1, 100000);
    Init_LCD();
    clear_LCD();
#endif

    Nabto_Test();

    while (1) {             // Something wrong on going here.
        PB15 = !PB15;       // Blink red LED to indicate error.
        delay_ms(1000);
    }
}
