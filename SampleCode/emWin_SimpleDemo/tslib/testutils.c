/*
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: testutils.c,v 1.2 2004/10/19 22:01:27 dlowder Exp $
 *
 * Waits for the screen to be touched, averages x and y sample
 * coordinates until the end of contact
 */
/*
  2020/06/22, move the APIs function sort_by_x(), sort_by_y(), and getxy()
              into the file fbutils.c.
          GUI APIs are changed into emWin GUI APIs.
*/
#include <stdio.h>
#include <stdlib.h>
#include "N9H30.h"
#include "fbutils.h"
#include "GUI.h"
#include "TouchPanel.h"


extern int ts_phy2log(int *sumx, int *sumy);
extern unsigned int xres, yres;

static int palette [] =
{
    /*0x000000, 0xffe080, 0xffffff, 0xe0c0a0, 0x304050, 0x80b8c0*/
    GUI_MAKE_COLOR(0x000000), GUI_MAKE_COLOR(0x80e0ff), GUI_MAKE_COLOR(0xffffff), GUI_MAKE_COLOR(0xa0c0e0), GUI_MAKE_COLOR(0x504030), GUI_MAKE_COLOR(0xc0b880),
    GUI_MAKE_COLOR(0x7F1F00), GUI_MAKE_COLOR(0x20201F), GUI_MAKE_COLOR(0x5F3F1F), GUI_MAKE_COLOR(0xAFBFCF), GUI_MAKE_COLOR(0xF080D0), GUI_MAKE_COLOR(0x3F477F),
    GUI_MAKE_COLOR(0x207820)
};
#define NR_COLORS (sizeof (palette) / sizeof (palette [0]))

#define NR_BUTTONS 3
struct ts_button
{
    int x, y, w, h;
    char *text;
    int flags;
#define BUTTON_ACTIVE 0x00000001
};
static struct ts_button buttons [NR_BUTTONS];

/* [inactive] border fill text [active] border fill text */
static int button_palette [6] =
{
    1, 4, 2,
    1, 5, 0
};

void button_draw (struct ts_button *button)
{
    int s = (button->flags & BUTTON_ACTIVE) ? 3 : 0;

    GUI_SetColor(palette[button_palette [s]]);
    GUI_DrawRect(button->x, button->y, button->x + button->w, button->y + button->h);
    GUI_SetColor(palette[button_palette [s+1]]);
    GUI_FillRect(button->x + 1, button->y + 1, button->x + button->w - 2, button->y + button->h - 2 );
    GUI_SetColor(palette[button_palette [s + 2]]);
    GUI_DispStringHCenterAt(button->text, button->x + button->w / 2, button->y + button->h / 2);
}

int button_handle (struct ts_button *button, int x, int y, unsigned int p)
{
    int inside = (x >= button->x) && (y >= button->y) &&
                 (x < button->x + button->w) &&
                 (y < button->y + button->h);

    if (p > 0)
    {
        if (inside)
        {
            if (!(button->flags & BUTTON_ACTIVE))
            {
                button->flags |= BUTTON_ACTIVE;
                button_draw (button);
            }
        }
        else if (button->flags & BUTTON_ACTIVE)
        {
            button->flags &= ~BUTTON_ACTIVE;
            button_draw (button);
        }
    }
    else if (button->flags & BUTTON_ACTIVE)
    {
        button->flags &= ~BUTTON_ACTIVE;
        button_draw (button);
        return 1;
    }

    return 0;
}

static void refresh_screen(void)
{
    int i;

    //fillrect (0, 0, xres - 1, yres - 1, 0);
    GUI_Clear();
//    put_string_center (xres/2, yres/4,   "Touchscreen test program", 1);
//    put_string_center (xres/2, yres/4+20,"Touch screen to move crosshair", 2);

    GUI_SetColor(palette[1]);
    GUI_DispStringHCenterAt("Touchscreen test program", xres / 2, yres / 4);

    GUI_SetColor(palette[2]);
    GUI_DispStringHCenterAt("Touch screen to move crosshair", xres / 2, yres / 4 + 20);

    for (i = 0; i < NR_BUTTONS; i++)
        button_draw (&buttons [i]);
}

int ts_test(int xsize, int ysize)
{
    int sumx, sumy;
    int x, y;
    unsigned int i;
    unsigned int mode = 0;
    int quit_pressed = 0;
    GUI_DRAWMODE defmode;

    xres = xsize;
    yres = ysize;

    x = xres/2;
    y = yres/2;

//   for (i = 0; i < NR_COLORS; i++)
//       setcolor (i, palette [i]);

    /* Initialize buttons */
    //memset (&buttons, 0, sizeof (buttons));
    buttons [0].w = buttons [1].w = buttons [2].w = xres / 4;
    buttons [0].h = buttons [1].h = buttons [2].h = 20;
    buttons [0].x = 0;
    buttons [1].x = (3 * xres) / 8;
    buttons [2].x = (3 * xres) / 4 - 1;
    buttons [0].y = buttons [1].y = buttons [2].y = 10;
    buttons [0].text = "Drag";
    buttons [1].text = "Draw";
    buttons [2].text = "Quit";

    refresh_screen ();

    while (1)
    {
        struct ts_sample samp;

        /* Show the cross */
        if ((mode & 15) != 1)
        {
            defmode = GUI_GetDrawMode();
            GUI_SetDrawMode(GUI_DRAWMODE_XOR);
            put_cross(x, y); // 2 | XORMODE);
            GUI_SetDrawMode(defmode);
        }

        if ( Read_TouchPanel(&sumx, &sumy) > 0)
        {
            ts_phy2log(&sumx, &sumy);
            samp.x = sumx;
            samp.y = sumy;
            samp.pressure = 1000;
        }
        else
        {
            samp.x = x;
            samp.y = y;
            samp.pressure = 0;
        }
        GUI_Delay(10);

        /* Hide it */
        if ((mode & 15) != 1)
        {
            defmode = GUI_GetDrawMode();
            GUI_SetDrawMode(GUI_DRAWMODE_XOR);
            put_cross(x, y); // 2 | XORMODE);
            GUI_SetDrawMode(defmode);
        }

        for (i = 0; i < NR_BUTTONS; i++)
            if (button_handle(&buttons [i], samp.x, samp.y, samp.pressure))
                switch (i)
                {
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

        if (samp.pressure > 0)
        {
            if (mode == 0x80000001)
            {
                GUI_DrawLine(x, y, samp.x, samp.y); //, 2);
            }
            //pixel(x, y, 2);
            x = samp.x;
            y = samp.y;
            mode |= 0x80000000;
        }
        else
            mode &= ~0x80000000;
        if (quit_pressed)
            break;
    }
    //fillrect(0, 0, xres - 1, yres - 1, 0);
    GUI_Clear();

    return 0;
}
