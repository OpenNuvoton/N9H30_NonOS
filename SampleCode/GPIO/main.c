/**************************************************************************//**
 * @file     main.c
 * @brief    N9H30 Driver Sample Code
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "N9H30.h"
#include "sys.h"
#include "gpio.h"

/*-----------------------------------------------------------------------------*/
volatile int eint_complete=0;
INT32 EINT0Callback(UINT32 status, UINT32 userData)
{
    /* To do */
    eint_complete=1;

    /********/
    GPIO_ClrISRBit(GPIOH,BIT0);
    return 0;
}

volatile int gpio_complete=0;
INT32 GPIODCallback(UINT32 status, UINT32 userData)
{
    /* To do */
    if(status & BIT3)
      gpio_complete=1;
    /********/
    GPIO_ClrISRBit(GPIOD,status);
    return 0;
}

int main(void)
{
    int32_t i32Err;
    *((volatile unsigned int *)REG_AIC_MDCR)=0xFFFFFFFF;  // disable all interrupt channel
    *((volatile unsigned int *)REG_AIC_MDCRH)=0xFFFFFFFF;  // disable all interrupt channel

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    sysprintf("+-------------------------------------------------+\n");
    sysprintf("|                 GPIO Sample Code                |\n");
    sysprintf("+-------------------------------------------------+\n\n");

    /* Configure Port B to input mode and pull-up */
    GPIO_Open(GPIOB, DIR_INPUT, PULL_UP);
   
    /* Set Port B output data to 0xFFF */
    GPIO_Set(GPIOB, 0xFFF);

    /* Set Port B output data to 0x000 */
    GPIO_Clr(GPIOB, 0xFFF);

    /* Configure Port B to default value */
    GPIO_Close(GPIOB);

    i32Err = 0;
    sysprintf("GPIO PD.3(output mode) connect to PD.4(input mode) ......");

    /* Configure PD3 to output mode */
    GPIO_OpenBit(GPIOD, BIT3, DIR_OUTPUT, NO_PULL_UP);

    /* Configure PD4 to output mode */
    GPIO_OpenBit(GPIOD, BIT4, DIR_INPUT, NO_PULL_UP);

    /* Use Pin Data Input/Output Control to pull specified I/O or get I/O pin status */
    /* Pull PD.3 to High and check PD.4 status */
    GPIO_SetBit(GPIOD, BIT3);

    if(GPIO_ReadBit(GPIOD,BIT4)==0)  
      i32Err = 1;

    /* Pull PD.3 to Low and check PD.4 status */
    GPIO_ClrBit(GPIOD, BIT3);

    if(GPIO_ReadBit(GPIOD,BIT4)==1)  
      i32Err = 1;

    if(i32Err)
    {
        sysprintf("  [FAIL].\n");
    }
    else
    {
        sysprintf("  [OK].\n");
    }

    /* Configure PD3 to default value */
    GPIO_CloseBit(GPIOD, BIT3);

    /* Configure PD4 to default value */
    GPIO_CloseBit(GPIOD, BIT4);

    /* Set MFP_GPH0 to EINT0 */
    outpw(REG_SYS_GPH_MFPL,(inpw(REG_SYS_GPH_MFPL) & ~(0xF<<0)) | (0xF<<0));

    /* Configure PH0 to input mode and pull-up */
    GPIO_OpenBit(GPIOH, BIT0, DIR_INPUT, PULL_UP);

    /* Confingure PH0 to rising-edge trigger */
    GPIO_EnableTriggerType(GPIOH, BIT0,RISING);

    /* Enable external 0 interrupt */
    GPIO_EnableEINT(NIRQ0, (GPIO_CALLBACK)EINT0Callback, 0);

    /* waiting for external 0 interrupt */
    sysprintf("waiting for PH0 rsing-edge trigger...");
    while(!eint_complete);

    /* Disable PH0 trigger type */
    GPIO_DisableTriggerType(GPIOH, BIT0);

    /* Enable external 0 interrupt */
    GPIO_DisableEINT(NIRQ0);

    sysprintf("  [OK].\n");

    /* Configure PH0 to default value */
    GPIO_CloseBit(GPIOH, BIT0);


    /* Configure PD3 to output mode */
    GPIO_OpenBit(GPIOD, BIT3, DIR_INPUT, NO_PULL_UP);

    /* Confingure PD3 to falling-edge trigger */
    GPIO_EnableTriggerType(GPIOD, BIT3,FALLING);

    /* Enable GPIOD interrupt */
    GPIO_EnableInt(GPIOD, (GPIO_CALLBACK)GPIODCallback, 0);

    /* waiting for PD3 interrupt */
    sysprintf("waiting for PD3 falling-edge trigger...");
    while(!gpio_complete);

    /* Disable PD3 to trigger type */
    GPIO_DisableTriggerType(GPIOD, BIT3);

    /* Disable GPIOD interrupt */
    GPIO_DisableInt(GPIOD);

    /* Configure PD3 to default value */
    GPIO_CloseBit(GPIOD, BIT3);

    sysprintf("  [OK].\n");

    while(1);
}
