/*
 * Copyright (C) 2018 V4 Games
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Implements quest log.
 *
 * =======================================================================
 */

#include "header/client.h"

void CL_ParseQuestlog(void)
{
    int i, cnt, sel;

    sel = MSG_ReadByte(&net_message);
    cnt = MSG_ReadByte(&net_message);

    for (i = 0; i < cnt; i++)
    {
        char *quest = MSG_ReadString(&net_message);

        Q_strlcpy(cl.quest_names[i], quest, strlen(quest)+1);
    }

    cl.quest_max = cnt;
    cl.quest_selected = sel;
}

static void
Quest_DrawStringScaled(int x, int y, char *string, float factor)
{
    while (*string)
    {
        Draw_CharScaled(x, y, *string, factor);
        x += factor * 8;
        string++;
    }
}

static void
SetStringHighBit(char *s)
{
    while (*s)
    {
        *s++ |= 128;
    }
}

#define DISPLAY_QUESTS 8

void CL_DrawQuestlog(void)
{
    int i;
    char string[1024];
    int x, y;
    int selected;

    selected = cl.quest_selected;

    float scale = SCR_GetHUDScale();

    x = (viddef.width - scale * 256) / 2;
    y = (viddef.height - scale * 240) / 2;

    /* repaint everything next frame */
    SCR_DirtyScreen();

    Draw_PicScaled(x, y + scale * 8, "inventory", scale);

    y += scale * 24;
    x += scale * 24;

    Quest_DrawStringScaled(x, y, "quest name", scale);
    Quest_DrawStringScaled(x, y + scale * 8, "----------", scale);

    y += scale * 16;

    for (i = 0; i < cl.quest_max; i++)
    {
        Com_sprintf(string, sizeof(string), "%s", cl.quest_names[i]);

        if (i != selected)
        {
            SetStringHighBit(string);
        }
        else
        {
            /* draw a blinky cursor by the selected quest */
            if ((int)(cls.realtime * 10) & 1)
            {
                Draw_CharScaled(x - scale * 8, y, 15, scale);
            }
        }

        Quest_DrawStringScaled(x, y, string, scale);

        y += scale * 8;
    }
}
