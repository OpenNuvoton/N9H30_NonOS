#include "N9H30.h"

#include "sys.h"
#include "spi.h"

#define TIMEOUT  1000000 /* unit of timeout is micro second */
#define ERR_TIMEOUT  (-1)
#define ERR_ALIGNMENT  (-2)
#define BLOCK_SIZE 0x10000 /* 64 KB */
#define SECTOR_SIZE 0x1000 /* 4 KB */
#define PAGE_SIZE 256

void _DemoSpiInit(void)
{
    /* Configure multi function pins to SPI0 */
    outpw(REG_SYS_GPB_MFPL, (inpw(REG_SYS_GPB_MFPL) & ~0xff000000) | 0xBB000000);
    outpw(REG_SYS_GPB_MFPH, (inpw(REG_SYS_GPB_MFPH) & ~0xff) | 0xBB);

    spiInit(0);
    spiOpen(0);

    // set spi interface speed to 2MHz
    spiIoctl(0, SPI_IOC_SET_SPEED, 2000000, 0);
    // set transfer length to 8-bit
    spiIoctl(0, SPI_IOC_SET_TX_BITLEN, 8, 0);
    // set transfer mode to mode-0
    spiIoctl(0, SPI_IOC_SET_MODE, 0, 0);
}

void SpiFlash_ChipErase(void)
{
    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x06, Write enable
    spiWrite(0, 0, 0x06);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    //////////////////////////////////////////

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0xC7, Chip Erase
    spiWrite(0, 0, 0xC7);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);
}

int SpiFlash_SectorErase(uint32_t StartAddress)
{
    /* Check if sector alignment */
    if (StartAddress % SECTOR_SIZE)
    {
        sysprintf("\nErase address 0x%x is not 0x%x alignment!\n",StartAddress,SECTOR_SIZE);
        return ERR_ALIGNMENT;
    }

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x06, Write enable
    spiWrite(0, 0, 0x06);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    //////////////////////////////////////////

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x20, Sector Erase
    spiWrite(0, 0, 0x20);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    // send 24-bit start address
    spiWrite(0, 0, (StartAddress>>16) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    spiWrite(0, 0, (StartAddress>>8) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    spiWrite(0, 0, StartAddress & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }


    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    return 0;
}


uint8_t SpiFlash_ReadStatusReg(void)
{
    uint8_t u8Status;

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x05, Read status register
    spiWrite(0, 0, 0x05);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // read status
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    u8Status = spiRead(0, 0);

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    return u8Status;
}

void SpiFlash_WaitReady(void)
{
    uint8_t ReturnValue;

    do
    {
        ReturnValue = SpiFlash_ReadStatusReg();
        ReturnValue = ReturnValue & 1;
    }
    while(ReturnValue!=0);   // check the BUSY bit
}

static int SpiFlash_NormalPageProgram(uint32_t StartAddress, uint8_t *u8DataBuffer)
{
    uint32_t i = 0;

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x06, Write enable
    spiWrite(0, 0, 0x06);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);


    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x02, Page program
    spiWrite(0, 0, 0x02);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    // send 24-bit start address
    spiWrite(0, 0, (StartAddress>>16) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    spiWrite(0, 0, (StartAddress>>8) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    spiWrite(0, 0, StartAddress & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    // write data
    for(i=0; i<28; i++)
    {
        spiWrite(0, 0, u8DataBuffer[i]);
        spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
        while(spiGetBusyStatus(0));
    }

    spiWrite(0, 0, 0x5A);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    spiWrite(0, 0, 0xA5);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    spiWrite(0, 0, 0xAA);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    spiWrite(0, 0, 0x55);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    return 0;
}

int SpiFlash_NormalProgram(uint32_t StartAddress, uint8_t *u8DataBuffer, uint32_t data_len)
{
    uint32_t i = 0;
    uint32_t page_no;

    /* Check if page alignment */
    if (StartAddress % PAGE_SIZE)
    {
        sysprintf("\nProgram address 0x%x is not 0x%x alignment!\n",StartAddress,PAGE_SIZE);
        return ERR_ALIGNMENT;
    }

    page_no = data_len/PAGE_SIZE;
    if (data_len % PAGE_SIZE)
        page_no++;

    for (i = 0; i < page_no; i++)
    {
        SpiFlash_NormalPageProgram((StartAddress + (PAGE_SIZE*i)), (u8DataBuffer + (PAGE_SIZE*i) ));
        SpiFlash_WaitReady();
    }

    return 0;
}


static int SpiFlash_NormalPageRead(uint32_t StartAddress, uint8_t *u8DataBuffer)
{
    uint32_t i;

    /* Check if page alignment */
    if (StartAddress % PAGE_SIZE)
    {
        sysprintf("\nRead address 0x%x is not 0x%x alignment!\n",StartAddress,PAGE_SIZE);
        return ERR_ALIGNMENT;
    }

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x03, Read data
    spiWrite(0, 0, 0x03);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    // send 24-bit start address
    spiWrite(0, 0, (StartAddress>>16) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    spiWrite(0, 0, (StartAddress>>8) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    spiWrite(0, 0, StartAddress & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0))
    {
        ;
    }

    // read data
    for(i=0; i < PAGE_SIZE; i++)
    {
        spiWrite(0, 0, 0x00);
        spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
        while(spiGetBusyStatus(0))
        {
            ;
        }
        u8DataBuffer[i] = spiRead(0, 0);
    }

    return 0;
}

int SpiFlash_NormalRead(uint32_t StartAddress, uint8_t *u8DataBuffer, uint32_t data_len)
{
    uint32_t i;
    uint32_t page_no;
    uint8_t *pData = u8DataBuffer;

    /* Check if page alignment */
    if (StartAddress % PAGE_SIZE)
    {
        sysprintf("\nRead address 0x%x is not 0x%x alignment!\n",StartAddress,PAGE_SIZE);
        return ERR_ALIGNMENT;
    }

    page_no = data_len/PAGE_SIZE;
    if (data_len % PAGE_SIZE)
        page_no++;

    // read data
    for(i=0; i < page_no; i++)
    {
        SpiFlash_NormalPageRead(StartAddress, pData);
        SpiFlash_WaitReady();
        StartAddress += PAGE_SIZE;
        pData += PAGE_SIZE;
    }

    return 0;
}


uint16_t SpiFlash_ReadMidDid(void)
{
    uint8_t u8RxData[2];

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x90, Read Manufacturer/Device ID
    spiWrite(0, 0, 0x90);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // send 24-bit '0', dummy
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // receive 16-bit
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    u8RxData[0] = spiRead(0, 0);

    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    u8RxData[1] = spiRead(0, 0);

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    return ( (u8RxData[0]<<8) | u8RxData[1] );
}
