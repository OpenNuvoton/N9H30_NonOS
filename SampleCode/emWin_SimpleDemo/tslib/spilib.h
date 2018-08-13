#ifndef __SPILIB_H__
#define __SPILIB_H__

#define __DEMO_TSFILE_ADDR__    0x00200000 /* SPI flash 2MB address */

void _DemoSpiInit(void);
void SpiFlash_EraseSector(uint32_t u32Addr);
uint8_t SpiFlash_ReadStatusReg(void);
void SpiFlash_WaitReady(void);
void SpiFlash_NormalPageProgram(uint32_t StartAddress, uint8_t *u8DataBuffer);
void SpiFlash_NormalRead(uint32_t StartAddress, uint8_t *u8DataBuffer);
uint16_t SpiFlash_ReadMidDid(void);

#endif
