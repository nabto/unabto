;/******************************************************************************
; * @file     startup_Nano1X2Series.s
; * @version  V1.00
; * $Revision: 5 $
; * $Date: 14/08/04 3:42p $ 
; * @brief    CMSIS ARM Cortex-M0 Core Device Startup File
; *
; * @note
; * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
;*****************************************************************************/  

    MODULE  ?cstartup

    ;; Forward declaration of sections.
    SECTION CSTACK:DATA:NOROOT(3) ;; 8 bytes alignment

    SECTION .intvec:CODE:NOROOT(2);; 4 bytes alignment

    EXTERN  __iar_program_start
    EXTERN  HardFault_Handler
    PUBLIC  __vector_table

    DATA
__vector_table
    DCD     sfe(CSTACK)
    DCD     __iar_program_start

    DCD     NMI_Handler
    DCD     HardFault_Handler
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     0
    DCD     SVC_Handler
    DCD     0
    DCD     0
    DCD     PendSV_Handler
    DCD     SysTick_Handler

    ; External Interrupts
    DCD     BOD_IRQHandler            ; Brownout low voltage detected interrupt
    DCD     WDT_IRQHandler            ; Watch Dog Timer interrupt
    DCD     EINT0_IRQHandler          ; External signal interrupt from PB.14 pin
    DCD     EINT1_IRQHandler          ; External signal interrupt from PB.15 pin
    DCD     GPABC_IRQHandler          ; External interrupt from PA[15:0]/PB[15:0]/PC[15:0]
    DCD     GPDEF_IRQHandler          ; External interrupt from PD[15:0]/PE[15:0]/PF[7:0]
    DCD     PWM0_IRQHandler           ; PWM 0 interrupt
    DCD     0                         ; Reserved
    DCD     TMR0_IRQHandler           ; Timer 0 interrupt
    DCD     TMR1_IRQHandler           ; Timer 1 interrupt
    DCD     TMR2_IRQHandler           ; Timer 2 interrupt
    DCD     TMR3_IRQHandler           ; Timer 3 interrupt
    DCD     UART0_IRQHandler          ; UART0 interrupt
    DCD     UART1_IRQHandler          ; UART1 interrupt
    DCD     SPI0_IRQHandler           ; SPI0 interrupt
    DCD     SPI1_IRQHandler           ; SPI1 interrupt
    DCD     Default_Handler           ; Reserved
    DCD     HIRC_IRQHandler           ; HIRC interrupt
    DCD     I2C0_IRQHandler           ; I2C0 interrupt
    DCD     I2C1_IRQHandler           ; I2C1 interrupt
    DCD     Default_Handler           ; Reserved
    DCD     SC0_IRQHandler            ; SC0 interrupt
    DCD     SC1_IRQHandler            ; SC1 interrupt
    DCD     ACMP_IRQHandler           ; ACMP interrupt
    DCD     Default_Handler           ; Reserved
    DCD     LCD_IRQHandler            ; LCD interrupt
    DCD     PDMA_IRQHandler           ; PDMA interrupt
    DCD     Default_Handler           ; Reserved
    DCD     PDWU_IRQHandler           ; Power Down Wake up interrupt
    DCD     ADC_IRQHandler            ; ADC interrupt
    DCD     Default_Handler           ; Reserved
    DCD     RTC_IRQHandler            ; Real time clock interrupt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
    THUMB
    PUBWEAK Reset_Handler
    SECTION .text:CODE:REORDER(2)       ; 4 bytes alignment
Reset_Handler
        LDR     R0, =0x50000100
        ; Unlock Register
        LDR     R1, =0x59
        STR     R1, [R0]
        LDR     R1, =0x16
        STR     R1, [R0]
        LDR     R1, =0x88
        STR     R1, [R0]

        ; Init POR
        LDR     R2, =0x50000060
        LDR     R1, =0x00005AA5
        STR     R1, [R2]

        ; Lock register
        MOVS    R1, #0
        STR     R1, [R0]

        LDR      R0, =__iar_program_start
        BX       R0

    PUBWEAK NMI_Handler
    PUBWEAK SVC_Handler
    PUBWEAK PendSV_Handler
    PUBWEAK SysTick_Handler
    PUBWEAK BOD_IRQHandler
    PUBWEAK WDT_IRQHandler
    PUBWEAK EINT0_IRQHandler
    PUBWEAK EINT1_IRQHandler
    PUBWEAK GPABC_IRQHandler
    PUBWEAK GPDEF_IRQHandler
    PUBWEAK PWM0_IRQHandler
    PUBWEAK TMR0_IRQHandler
    PUBWEAK TMR1_IRQHandler
    PUBWEAK TMR2_IRQHandler
    PUBWEAK TMR3_IRQHandler
    PUBWEAK UART0_IRQHandler
    PUBWEAK UART1_IRQHandler
    PUBWEAK SPI0_IRQHandler
    PUBWEAK SPI1_IRQHandler
    PUBWEAK HIRC_IRQHandler
    PUBWEAK I2C0_IRQHandler
    PUBWEAK I2C1_IRQHandler
    PUBWEAK SC0_IRQHandler
    PUBWEAK SC1_IRQHandler
    PUBWEAK LCD_IRQHandler
    PUBWEAK PDMA_IRQHandler
    PUBWEAK PDWU_IRQHandler
    PUBWEAK ADC_IRQHandler
    PUBWEAK ACMP_IRQHandler
    PUBWEAK RTC_IRQHandler

    SECTION .text:CODE:REORDER(2)

NMI_Handler
SVC_Handler
PendSV_Handler
SysTick_Handler
BOD_IRQHandler
WDT_IRQHandler
EINT0_IRQHandler
EINT1_IRQHandler
GPABC_IRQHandler
GPDEF_IRQHandler
PWM0_IRQHandler
TMR0_IRQHandler
TMR1_IRQHandler
TMR2_IRQHandler
TMR3_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
HIRC_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
SC0_IRQHandler
SC1_IRQHandler
LCD_IRQHandler
PDMA_IRQHandler
PDWU_IRQHandler
ADC_IRQHandler
ACMP_IRQHandler
RTC_IRQHandler
Default_Handler
    B Default_Handler






    END
;/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
