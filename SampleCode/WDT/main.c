/**************************************************************************//**
 * @file     main.c
 * @brief    N9H30 WDT Sample Code
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "N9H30.h"
#include "sys.h"


void WDT_IRQHandler(void)
{
    // Reload WWDT counter and clear WWDT interrupt flag
    sysClearWatchDogTimerCount();
    sysClearWatchDogTimerInterruptStatus();
    sysprintf("Reset WDT counter\n");

}

int main(void)
{
    // Disable all interrupts.
    outpw(REG_AIC_MDCR, 0xFFFFFFFE);
    outpw(REG_AIC_MDCRH, 0x3FFFFFFF);

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    sysprintf("\nThis sample code demonstrate reset WDT function\n");

    // Disable write protect mode to control WDT register
    outpw(REG_SYS_REGWPCTL,0x59);
    outpw(REG_SYS_REGWPCTL,0x16);
    outpw(REG_SYS_REGWPCTL,0x88);
    while(!(inpw(REG_SYS_REGWPCTL)&(1<<0)));

    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 1); // Enable WDT engine clock

    sysSetWatchDogTimerInterval(6);     // Set WDT time out interval to 2^16 Twdt = 0.7 sec. Where Twdt = 12MHZ / 128
    sysInstallWatchDogTimerISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, WDT_IRQHandler);
    sysEnableWatchDogTimerReset();
    sysEnableWatchDogTimer();

    while(1);
}
