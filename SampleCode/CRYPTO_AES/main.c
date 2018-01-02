/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/06 9:54a $
 * @brief    N9H30 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "N9H30.h"
#include "crypto.h"


uint32_t au32MyAESKey[8] = {
    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f
};

uint32_t au32MyAESIV[4] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};


__align(32) uint8_t au8InputData_Pool[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};
uint8_t  *au8InputData;


__align(32) uint8_t au8OutputData_Pool[1024];
uint8_t  *au8OutputData;


static volatile int  g_AES_done;

void CRYPTO_IRQHandler()
{
    if (AES_GET_INT_FLAG()) {
        g_AES_done = 1;
        AES_CLR_INT_FLAG();
    }
}


void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0) {
        sysprintf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            sysprintf("%02x ", pucBuff[nIdx + i]);
        sysprintf("  ");
        for (i = 0; i < 16; i++) {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                sysprintf("%c", pucBuff[nIdx + i]);
            else
                sysprintf(".");
            nBytes--;
        }
        nIdx += 16;
        sysprintf("\n");
    }
    sysprintf("\n");
}


/*-----------------------------------------------------------------------------*/
int main(void)
{
	int   data_len;
	
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    /* enable Crypto clock */
	outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | (1 << 23));

    sysprintf("+------------------------------------+\n");
    sysprintf("|     Crypto AES Sample Program      |\n");
    sysprintf("+------------------------------------+\n");

	sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, CRPT_IRQn, (PVOID)CRYPTO_IRQHandler);
  	sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(CRPT_IRQn);
	
	au8InputData = (uint8_t *)((uint32_t)au8InputData_Pool | 0x80000000);
	au8OutputData = (uint8_t *)((uint32_t)au8OutputData_Pool | 0x80000000);

	data_len = sizeof(au8InputData_Pool);

    AES_ENABLE_INT();

	sysprintf("\n\n[Plain text] =>\n");
    dump_buff_hex(au8InputData, data_len);

    /*---------------------------------------
     *  AES-128 ECB mode encrypt
     *---------------------------------------*/
    AES_Open(0, 1, AES_MODE_ECB, AES_KEY_SIZE_128, AES_IN_OUT_SWAP);
    AES_SetKey(0, au32MyAESKey, AES_KEY_SIZE_128);
    AES_SetInitVect(0, au32MyAESIV);
    AES_SetDMATransfer(0, (uint32_t)au8InputData, (uint32_t)au8OutputData, data_len);

    g_AES_done = 0;
    AES_Start(0, CRYPTO_DMA_ONE_SHOT);
    while (!g_AES_done);

    sysprintf("AES encrypt done.\n\n");

	sysprintf("[Cypher text] =>\n");
    dump_buff_hex(au8OutputData, data_len);

    memset(au8InputData, 0, data_len);     /* To prove it, clear plain text data. */

    /*---------------------------------------
     *  AES-128 ECB mode decrypt
     *---------------------------------------*/
    AES_Open(0, 0, AES_MODE_ECB, AES_KEY_SIZE_128, AES_IN_OUT_SWAP);
    AES_SetKey(0, au32MyAESKey, AES_KEY_SIZE_128);
    AES_SetInitVect(0, au32MyAESIV);
    AES_SetDMATransfer(0, (uint32_t)au8OutputData, (uint32_t)au8InputData, data_len);

    g_AES_done = 0;
    AES_Start(0, CRYPTO_DMA_ONE_SHOT);
    while (!g_AES_done);

    sysprintf("AES decrypt done.\n\n");

	sysprintf("[Cypher text back to plain text] =>\n");
    dump_buff_hex(au8InputData, data_len);
    
    sysprintf("AES test done.\n");

    return 0;
}
