/**************************************************************************//**
 * @file     main.c
 * @brief    Use internal SRAM as back end storage media to simulate a 
 *           30 KB USB pen drive
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "N9H30.h"
#include "sys.h"
#include "usbd.h"
#include "massstorage.h"


/*--------------------------------------------------------------------------*/
extern uint8_t volatile g_u8MscStart;
extern void USBD_IRQHandler(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	sysInitializeUART();
    sysprintf("\n");
    sysprintf("=================================\n");
    sysprintf("      N9H30 USB Mass Storage     \n");
    sysprintf("=================================\n");

	sysDisableCache();
	sysInvalidCache();
	sysSetMMUMappingMethod(MMU_DIRECT_MAPPING);
	sysEnableCache(CACHE_WRITE_BACK);

    sysInstallISR(HIGH_LEVEL_SENSITIVE|IRQ_LEVEL_1, USBD_IRQn, (PVOID)USBD_IRQHandler);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);

    USBD_Open(&gsInfo, MSC_ClassRequest, NULL);

    /* Endpoint configuration */
    MSC_Init();
	sysEnableInterrupt(USBD_IRQn);

    /* Start transaction */
    while(1) {
        if (USBD_IS_ATTACHED()) {
            USBD_Start();
            break;
        }
    }

    while(1) {
        if (g_u8MscStart)
            MSC_ProcessCmd();
    }
}



/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/

