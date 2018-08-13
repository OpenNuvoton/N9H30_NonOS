#ifndef __TOUCHPANEL_H__
#define __TOUCHPANEL_H__

#define __DEMO_TS_WIDTH__       800
#define __DEMO_TS_HEIGHT__      480

int Init_TouchPanel(void);
int Read_TouchPanel(int *x, int *y);
int Uninit_TouchPanel(void);
int Check_TouchPanel(void);
#endif
