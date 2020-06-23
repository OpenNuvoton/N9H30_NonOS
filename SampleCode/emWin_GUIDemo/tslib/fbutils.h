/*
 * fbutils.h
 *
 * Headers for utility routines for framebuffer interaction
 *
 * Copyright 2002 Russell King and Doug Lowder
 *
 * This file is placed under the GPL.  Please see the
 * file COPYING for details.
 *
 */

#ifndef _FBUTILS_H
#define _FBUTILS_H
struct ts_sample
{
    int     x;
    int     y;
    unsigned int    pressure;
};

typedef struct
{
    int x[5], xfb[5];
    int y[5], yfb[5];
    int a[7];
} calibration;

void put_cross(int x, int y);
int perform_calibration(calibration *cal);
void get_sample (calibration *cal,int index, int x, int y, char *name);

#endif /* _FBUTILS_H */
