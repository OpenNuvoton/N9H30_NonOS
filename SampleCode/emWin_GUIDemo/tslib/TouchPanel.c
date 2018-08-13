

#include "stdlib.h"
#include "N9H30.h"
#include "adc.h"
#include "TouchPanel.h"

//#define XSIZE_PHYS        800
//#define YSIZE_PHYS        480

INT32 TouchXYCallback(UINT32 status, UINT32 userData)
{
    /*  The status content that contains Touch x-position and touch y-position.
     *  X-position = (status & 0xFFF);
     *  Y-position = ((status>>16) & 0xFFF);
     */
    outpw(0xB8003000+0xC4,!inpw(0xB8003000+0xC4));
    return 0;
}

INT32 TouchZCallback(UINT32 status, UINT32 userData)
{
    /*  The status content that contains touch pressure measure Z1 and touch pressure measure Z2.
     *  Pressure measure Z1 = (status & 0xFFF);
     *  Pressure measure Z2 = ((status>>16) & 0xFFF);
     */
    return 0;
}

volatile int pendown_complete=0;
INT32 PenDownCallback(UINT32 status, UINT32 userData)
{
    pendown_complete=1;
    adcIoctl(PEPOWER_OFF,0,0);
    return 0;
}


int Init_TouchPanel(void)
{
    int userdata=0;
//  adc_init();
//  adc_open(ADC_TS_4WIRE, XSIZE_PHYS, YSIZE_PHYS);  //320x240
    adcOpen();
    outpw(REG_ADC_CONF,(inpw(REG_ADC_CONF)&~(0xfful<<24))|0xfful<<24);
    adcIoctl(T_ON,(UINT32)TouchXYCallback,userdata);
    adcIoctl(Z_ON,(UINT32)TouchZCallback,userdata);
    adcIoctl(PEDEF_ON,(UINT32)PenDownCallback,userdata);
    pendown_complete=0;
    adcIoctl(PEPOWER_ON,0,0);
    return 1;
}

int z_th = 10;
int old_x, old_y;
int Read_TouchPanel(int *x, int *y)
{
//return 0 fai;ure, 1 success.
    short adc_x, adc_y,z1,z2;
    if(pendown_complete!=0)
    {
        adcIoctl(START_MST_POLLING,0,0);
        adcReadXY(&adc_x,&adc_y,1);
        adcReadZ(&z1,&z2,1);
        if( ((inpw(REG_ADC_ZSORT0)&0xfff)<=z_th) ||
                ((inpw(REG_ADC_ZSORT1)&0xfff)<=z_th) ||
                ((inpw(REG_ADC_ZSORT2)&0xfff)<=z_th) ||
                ((inpw(REG_ADC_ZSORT3)&0xfff)<=z_th)    )
        {
            //sysprintf("0000000000000\n");
            pendown_complete=0;
            adcIoctl(PEPOWER_ON,0,0);
            *x = old_x;
            *y = old_y;
        }
        else
        {
            //*x = (XSIZE_PHYS * (adc_x)) / 4096;
            *x = adc_x;
            //*y = (YSIZE_PHYS * (4095-adc_y)) / 4096;
            *y = adc_y;
            old_x = *x;
            old_y = *y;
        }

        return 1;
    }
    else
    {
        *x = old_x;
        *y = old_y;
        return 0;
    }
}

int Uninit_TouchPanel(void)
{
    adcClose();
    return 1;
}

int Check_TouchPanel(void)
{
#if 0
    if ( (inp32(REG_ADC_TSC) & ADC_UD) == ADC_UD)
        return 1;   //Pen down;
    else
        return 0;   //Pen up;
#else
    return 0;
#endif
}



int fsFileSeek()
{
    /* TODO */
    return 0;
}
int fsReadFile()
{
    /* TODO */
    return 0;
}
int fsWriteFile()
{
    /* TODO */
    return 0;
}


