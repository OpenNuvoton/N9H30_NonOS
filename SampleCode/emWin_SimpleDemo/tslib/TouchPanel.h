#ifndef __TOUCHPANEL_H__
#define __TOUCHPANEL_H__
int Init_TouchPanel(void);
int Read_TouchPanel(int *x, int *y);
int Uninit_TouchPanel(void);
int Check_TouchPanel(void);
#endif
