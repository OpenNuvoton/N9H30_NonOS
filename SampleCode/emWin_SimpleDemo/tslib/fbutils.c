/*
 * fbutils.c
 *
 * Utility routines for framebuffer interaction
 *
 * Copyright 2002 Russell King and Doug Lowder
 *
 * This file is placed under the GPL.  Please see the
 * file COPYING for details.
 *
 */
/*
  2020/06/22, add the APIs function sort_by_x(), sort_by_y(), and getxy() 
              from the file testutils.c.
	      add the APIs function perform_calibration() and get_sample() 
              from the file ts_calibrate.c.
*/

#include "stdlib.h"
#include "fbutils.h"
#include "GUI.h"

int ts_Read_TouchPanel(int *x, int *y);


void put_cross(int x, int y)
{
    GUI_DrawLine(x - 10, y, x - 2, y);
    GUI_DrawLine(x + 2, y, x + 10, y); 
    GUI_DrawLine(x, y - 10, x, y - 2); 
    GUI_DrawLine(x, y + 2, x, y + 10); 
    GUI_DrawLine(x - 6, y - 9, x - 9, y - 9); 
    GUI_DrawLine(x - 9, y - 8, x - 9, y - 6); 
    GUI_DrawLine(x - 9, y + 6, x - 9, y + 9); 
    GUI_DrawLine(x - 8, y + 9, x - 6, y + 9); 
    GUI_DrawLine(x + 6, y + 9, x + 9, y + 9); 
    GUI_DrawLine(x + 9, y + 8, x + 9, y + 6); 
    GUI_DrawLine(x + 9, y - 6, x + 9, y - 9); 
    GUI_DrawLine(x + 8, y - 9, x + 6, y - 9); 

}


int perform_calibration(calibration *cal)
{
    int j;
    float n, x, y, x2, y2, xy, z, zx, zy;
    float det, a, b, c, e, f, i;
    float scaling = 65536.0;

// Get sums for matrix
    n = x = y = x2 = y2 = xy = 0;
    for(j=0; j<5; j++)
    {
        n += 1.0;
        x += (float)cal->x[j];
        y += (float)cal->y[j];
        x2 += (float)(cal->x[j]*cal->x[j]);
        y2 += (float)(cal->y[j]*cal->y[j]);
        xy += (float)(cal->x[j]*cal->y[j]);
    }
// Get determinant of matrix -- check if determinant is too small
    det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
    if(det < 0.1 && det > -0.1)
    {
 //       sysprintf("ts_calibrate: determinant is too small -- %f\n",det);
        return 0;
    }

// Get elements of inverse matrix
    a = (x2*y2 - xy*xy)/det;
    b = (xy*y - x*y2)/det;
    c = (x*xy - y*x2)/det;
    e = (n*y2 - y*y)/det;
    f = (x*y - n*xy)/det;
    i = (n*x2 - x*x)/det;

// Get sums for x calibration
    z = zx = zy = 0;
    for(j=0; j<5; j++)
    {
        z += (float)cal->xfb[j];
        zx += (float)(cal->xfb[j]*cal->x[j]);
        zy += (float)(cal->xfb[j]*cal->y[j]);
    }

// Now multiply out to get the calibration for framebuffer x coord
    cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
    cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
    cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling));
#if 0 //close
    sysprintf("%f %f %f\n",(a*z + b*zx + c*zy),
              (b*z + e*zx + f*zy),
              (c*z + f*zx + i*zy));
#endif
// Get sums for y calibration
    z = zx = zy = 0;
    for(j=0; j<5; j++)
    {
        z += (float)cal->yfb[j];
        zx += (float)(cal->yfb[j]*cal->x[j]);
        zy += (float)(cal->yfb[j]*cal->y[j]);
    }

// Now multiply out to get the calibration for framebuffer y coord
    cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
    cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
    cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling));
#if 0  // closed
    sysprintf("%f %f %f\n",(a*z + b*zx + c*zy),
              (b*z + e*zx + f*zy),
              (c*z + f*zx + i*zy));
#endif
// If we got here, we're OK, so assign scaling to a[6] and return
    cal->a[6] = (int)scaling;
    return 1;
}

int sort_by_x(const void* a, const void *b)
{
    return (((struct ts_sample *)a)->x - ((struct ts_sample *)b)->x);
}

int sort_by_y(const void* a, const void *b)
{
    return (((struct ts_sample *)a)->y - ((struct ts_sample *)b)->y);
}




void getxy(int *x, int *y)
{
#define MAX_SAMPLES 128
    struct ts_sample samp[MAX_SAMPLES];
    int index, middle;
    int sumx, sumy;

 //   sysprintf("getxy\n");
again:
    do
    {
        if ( ts_Read_TouchPanel(&sumx, &sumy) > 0 )
        {
            if ( (sumx < 0) || ( sumy < 0 ) )
                continue;
            break;
        }
    }
    while (1);

    /* Now collect up to MAX_SAMPLES touches into the samp array. */
    index = 0;
    do
    {
        if (index < MAX_SAMPLES-1)
            index++;
        if ( ts_Read_TouchPanel(&sumx, &sumy) > 0)
        {
            samp[index].x = sumx;
            samp[index].y = sumy;
            samp[index].pressure = 1000;
        }
        else
        {
            samp[index].x = samp[index-1].x;
            samp[index].y = samp[index-1].y;
            samp[index].pressure = 0;
        }

//      sysprintf("%d %d %d\n", samp[index].x, samp[index].y , samp[index].pressure);
    }
    while (samp[index].pressure > 0);
 //   sysprintf("Took %d samples...\n",index);

    /*
     * At this point, we have samples in indices zero to (index-1)
     * which means that we have (index) number of samples.  We want
     * to calculate the median of the samples so that wild outliers
     * don't skew the result.  First off, let's assume that arrays
     * are one-based instead of zero-based.  If this were the case
     * and index was odd, we would need sample number ((index+1)/2)
     * of a sorted array; if index was even, we would need the
     * average of sample number (index/2) and sample number
     * ((index/2)+1).  To turn this into something useful for the
     * real world, we just need to subtract one off of the sample
     * numbers.  So for when index is odd, we need sample number
     * (((index+1)/2)-1).  Due to integer division truncation, we
     * can simplify this to just (index/2).  When index is even, we
     * need the average of sample number ((index/2)-1) and sample
     * number (index/2).  Calculate (index/2) now and we'll handle
     * the even odd stuff after we sort.
     */
    middle = index/2;
    if (x)
    {
        qsort(samp, index, sizeof(struct ts_sample), sort_by_x);
        if (index & 1)
            *x = samp[middle].x;
        else
            *x = (samp[middle-1].x + samp[middle].x) / 2;
    }
    if (y)
    {
        qsort(samp, index, sizeof(struct ts_sample), sort_by_y);
        if (index & 1)
            *y = samp[middle].y;
        else
            *y = (samp[middle-1].y + samp[middle].y) / 2;
    }
    if ( (index <= 3) || ( *x < 0) || ( *y < 0 ) )
        goto again;
}


void get_sample(calibration *cal,int index, int x, int y, char *name)
{
    static int last_x = -1, last_y;

    if (last_x != -1)
    {
#define NR_STEPS 10
        int dx = ((x - last_x) << 16) / NR_STEPS;
        int dy = ((y - last_y) << 16) / NR_STEPS;
        int i;
        last_x <<= 16;
        last_y <<= 16;
        for (i = 0; i < NR_STEPS; i++)
        {
            put_cross (last_x >> 16, last_y >> 16);
            //usleep (1000);
//          GUI_Delay(1);
            put_cross (last_x >> 16, last_y >> 16);
            last_x += dx;
            last_y += dy;
        }
    }


    put_cross(x, y);
    getxy (&cal->x [index], &cal->y [index]);
    put_cross(x, y);

    last_x = cal->xfb [index] = x;
    last_y = cal->yfb [index] = y;

//    sysprintf("%s : X = %4d Y = %4d\n", name, cal->x [index], cal->y [index]);
}
