/*
 *  tslib/src/ts_test.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 *
 * Basic test program for touchscreen library.
 */
//#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
//#include <sys/fcntl.h>
//#include <sys/ioctl.h>
//#include <sys/mman.h>
//#include <sys/time.h>
#include "tslib.h"
#include "fbutils.h"

extern int Read_TouchPanel(int *x, int *y);

static int palette [] =
{
	0x000000, 0xffe080, 0xffffff, 0xe0c0a0, 0x304050, 0x80b8c0
};
#define NR_COLORS (sizeof (palette) / sizeof (palette [0]))

struct ts_button {
	int x, y, w, h;
	char *text;
	int flags;
#define BUTTON_ACTIVE 0x00000001
};

/* [inactive] border fill text [active] border fill text */
static int button_palette [6] =
{
	1, 4, 2,
	1, 5, 0
};

#define NR_BUTTONS 3
static struct ts_button buttons [NR_BUTTONS];

static void button_draw (struct ts_button *button)
{
	int s = (button->flags & BUTTON_ACTIVE) ? 3 : 0;
	rect (button->x, button->y, button->x + button->w - 1,
	      button->y + button->h - 1, button_palette [s]);
	fillrect (button->x + 1, button->y + 1,
		  button->x + button->w - 2,
		  button->y + button->h - 2, button_palette [s + 1]);
	put_string_center (button->x + button->w / 2,
			   button->y + button->h / 2,
			   button->text, button_palette [s + 2]);
}

static int button_handle (struct ts_button *button, struct ts_sample *samp)
{
	int inside = (samp->x >= button->x) && (samp->y >= button->y) &&
		(samp->x < button->x + button->w) &&
		(samp->y < button->y + button->h);
	//sysprintf("samp->x=%d, samp->y=%d, button->x=%d, button->y=%d\n",samp->x,samp->y,button->x,button->y);
	if (samp->pressure > 0) {
		if (inside) {
			if (!(button->flags & BUTTON_ACTIVE)) {
				button->flags |= BUTTON_ACTIVE;
				button_draw (button);
			}
		} else if (button->flags & BUTTON_ACTIVE) {
			button->flags &= ~BUTTON_ACTIVE;
			button_draw (button);
		}
	} else if (button->flags & BUTTON_ACTIVE) {
		button->flags &= ~BUTTON_ACTIVE;
		button_draw (button);
                return 1;
	}

        return 0;
}

static void refresh_screen ()
{
	int i;

	fillrect (0, 0, xres - 1, yres - 1, 0);
	put_string_center (xres/2, yres/4,   "TSLIB test program", 1);
	put_string_center (xres/2, yres/4+20,"Touch screen to move crosshair", 2);

	for (i = 0; i < NR_BUTTONS; i++)
		button_draw (&buttons [i]);
}
extern int ts_phy2log(int *sumx, int *sumy);
int ts_TestMain(int xsize, int ysize)
{
//	struct tsdev *ts;
	int x, y;//old_x,old_y;
	unsigned int i;
	volatile unsigned int mode = 0;
	int quit_pressed = 0;

//	char *tsdevice=NULL;
	xres = xsize;
	yres = ysize;
	//x = xres/2;
	//y = yres/2;


	for (i = 0; i < NR_COLORS; i++)
		setcolor (i, palette [i]);

	/* Initialize buttons */
	memset (&buttons, 0, sizeof (buttons));
	buttons [0].w = buttons [1].w = buttons [2].w = xres / 4;
	buttons [0].h = buttons [1].h = buttons [2].h = 20;
	buttons [0].x = 0;
	buttons [1].x = (3 * xres) / 8;
	buttons [2].x = (3 * xres) / 4;
	buttons [0].y = buttons [1].y = buttons [2].y = 10;
	buttons [0].text = "Drag";
	buttons [1].text = "Draw";
	buttons [2].text = "Quit";

	refresh_screen ();

	while (1) {
		struct ts_sample samp;
		int ret;

		/* Show the cross */
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);

//		ret = ts_read(ts, &samp, 1);
		ret=Read_TouchPanel(&samp.x, &samp.y);
        ts_phy2log(&samp.x, &samp.y);

		/* Hide it */
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);

		if (samp.pressure==0 && ret != 1)
			continue;
			
		if(samp.pressure==1 && ret != 1){
			//samp.x = old_x;
			//samp.y = old_y;
			samp.pressure=0; 
		}else{
			//old_x = samp.x;
			//old_y = samp.y;
			samp.pressure=1;
		}
			
		for (i = 0; i < NR_BUTTONS; i++)
			if (button_handle (&buttons [i], &samp))
				switch (i) {
				case 0:
					mode = 0;
					refresh_screen ();
					break;
				case 1:
					mode = 1;
					refresh_screen ();
					break;
				case 2:
					quit_pressed = 1;
				}

//		sysprintf("%ld.%06ld: %6d %6d %6d\n", samp.tv.tv_sec, samp.tv.tv_usec,
//			samp.x, samp.y, samp.pressure);

		if (samp.pressure > 0) {
			if (mode == 0x80000001)
				line (x, y, samp.x, samp.y, 2);
			x = samp.x;
			y = samp.y;
			mode |= 0x80000000;
		} else
			mode &= ~0x80000000;
		if (quit_pressed)
			break;
	}
	//close_framebuffer();

	return 0;
}
