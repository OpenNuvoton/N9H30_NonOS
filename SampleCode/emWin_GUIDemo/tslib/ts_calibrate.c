/*
 *  tslib/tests/ts_calibrate.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_calibrate.c,v 1.8 2004/10/19 22:01:27 dlowder Exp $
 *
 * Basic test program for touchscreen library.
 */

/*
  2020/06/22  move the APIs function perform_calibration() and get_sample() 
              into the file fbutils.c
	      add the function ts_Read_TouchPanel()
*/

#include <stdio.h>
#include <stdlib.h>
#include "fbutils.h"
#include "N9H30.h"
#include "TouchPanel.h"

#ifdef __USE_SD__
#include "diskio.h"
#include "ff.h"
#else
#include "spilib.h"
#endif

#include "sys.h"
#include "GUI.h"


#ifdef __USE_SD__
extern FIL hFile;
#else
extern uint32_t * g_pu32Res;
#endif

int ts_writefile(void);
int ts_readfile(void);

int ts_calibrate(int xsize, int ysize);
int ts_phy2log(int *sumx, int *sumy);
int ts_Read_TouchPanel(int *x, int *y);

calibration cal, final_cal;

unsigned int xres, yres;

int ts_Read_TouchPanel(int *x, int *y)
{
	return(Read_TouchPanel(x, y));
}

int ts_calibrate(int xsize, int ysize)
{
    int i;
		GUI_DRAWMODE defmode;
    xres = xsize;
    yres = ysize;

    GUI_SetColor(0xffffe080);
    GUI_DispStringHCenterAt("TSLIB calibration utility", xres / 2, yres / 4);

    GUI_SetColor(0xffffffff);
    GUI_DispStringHCenterAt("Touch crosshair to calibrate", xres / 2, yres / 4 + 20);

    sysprintf("xres = %d, yres = %d\n", xres, yres);
		defmode = GUI_GetDrawMode();
	  GUI_SetDrawMode(GUI_DRAWMODE_XOR);
// Read a touchscreen event to clear the buffer
    //getxy(ts, 0, 0);

    get_sample (&cal, 0, 50,        50,        "Top left");
//  GUI_Delay(200);
    get_sample (&cal, 1, xres - 50, 50,        "Top right");
    //GUI_Delay(200);
    get_sample (&cal, 2, xres - 50, yres - 50, "Bot right");
//  GUI_Delay(200);
    get_sample (&cal, 3, 50,        yres - 50, "Bot left");
//  GUI_Delay(200);
    get_sample (&cal, 4, xres / 2,  yres / 2,  "Center");
//  GUI_Delay(200);

		GUI_SetDrawMode(defmode);
    if (perform_calibration (&cal))
    {
        sysprintf ("Calibration constants: ");
        for (i = 0; i < 7; i++) sysprintf("%d ", cal.a [i]);
        sysprintf("\n");
    }
    else
    {
        sysprintf("Calibration failed.\n");
        i = -1;
    }
    final_cal.a[0] = cal.a[1];
    final_cal.a[1] = cal.a[2];
    final_cal.a[2] = cal.a[0];
    final_cal.a[3] = cal.a[4];
    final_cal.a[4] = cal.a[5];
    final_cal.a[5] = cal.a[3];
    final_cal.a[6] = cal.a[6];
    return i;
}

int ts_phy2log(int *sumx, int *sumy)
{
    int xtemp,ytemp;

    xtemp = *sumx;
    ytemp = *sumy;
    *sumx = ( final_cal.a[2] +
              final_cal.a[0]*xtemp +
              final_cal.a[1]*ytemp ) / final_cal.a[6];
    *sumy = ( final_cal.a[5] +
              final_cal.a[3]*xtemp +
              final_cal.a[4]*ytemp ) / final_cal.a[6];
//sysprintf("After X=%d, Y=%d\n",*sumx, *sumy);
    return 1;
}

int ts_writefile(void)
{
#ifdef __USE_SD__
    size_t cnt;
    FRESULT res;
    res = f_lseek(&hFile, 0);
    if (res != FR_OK)
    {
        sysprintf("CANNOT seek the calibration into file\n");
        return -1;
    }

    res = f_write(&hFile, (char *)&final_cal.a[0], 28, &cnt);
    if (res != FR_OK)
    {
        sysprintf("CANNOT write the calibration into file, %d\n", cnt);
        return -1;
    }
#else
    SpiFlash_NormalProgram(__DEMO_TSFILE_ADDR__, (uint8_t *)&final_cal.a[0], 32);
#endif
    return 0;
}

int ts_readfile(void)
{
#ifdef __USE_SD__
    size_t cnt;
    FRESULT res;
    res = f_lseek(&hFile, 0);
    if (res != FR_OK)
    {
        sysprintf("CANNOT seek the calibration into file\n");
        return -1;
    }

    res = f_read(&hFile, (char *)&final_cal.a[0], 28, &cnt);
    if (res != FR_OK)
    {
        sysprintf("CANNOT read the calibration into file, %d\n", cnt);
        return -1;
    }
#else
    final_cal.a[0] = g_pu32Res[0];
    final_cal.a[1] = g_pu32Res[1];
    final_cal.a[2] = g_pu32Res[2];
    final_cal.a[3] = g_pu32Res[3];
    final_cal.a[4] = g_pu32Res[4];
    final_cal.a[5] = g_pu32Res[5];
    final_cal.a[6] = g_pu32Res[6];
#endif
    return 0;
}
