#ifndef __SPILIB_H__
#define __SPILIB_H__

#define __DEMO_TSFILE_ADDR__    0x00180000 /* SPI flash 1.5MB address */

void _DemoSpiInit(void);
int SpiFlash_SectorErase(uint32_t StartAddress);
uint8_t SpiFlash_ReadStatusReg(void);
void SpiFlash_WaitReady(void);
int SpiFlash_NormalProgram(uint32_t StartAddress, uint8_t *u8DataBuffer, uint32_t data_len);
int SpiFlash_NormalRead(uint32_t StartAddress, uint8_t *u8DataBuffer, uint32_t data_len);
uint16_t SpiFlash_ReadMidDid(void);

#endif
