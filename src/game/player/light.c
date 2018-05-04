/*
 * Copyright (C) 1997-2001 Id Software, Inc.
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
 * Flashlight logic.
 *
 * =======================================================================
 */

#include "../header/local.h"

void
FL_Toggle(edict_t *ent)
{
    vec3_t start, fwd, end, right;

    if (ent->flashlight)
    {
        G_FreeEdict(ent->flashlight);
        ent->flashlight = NULL;
        return;
    }

    AngleVectors(ent->client->v_angle, fwd, right, NULL);

    VectorSet(end, 100, 0, 0);
    G_ProjectSource(ent->s.origin, end, fwd, right, start);
    ent->flashlight = G_Spawn();
    ent->flashlight->owner = ent;
    ent->flashlight->movetype = MOVETYPE_NOCLIP;
    ent->flashlight->solid = SOLID_NOT;
    ent->flashlight->classname = "flashlight";
    ent->flashlight->s.modelindex = gi.modelindex("sprites/s_empty.sp2");
    ent->flashlight->s.skinnum = 0;
    ent->flashlight->s.effects |= EF_FLASHLIGHT;
    ent->flashlight->think = FL_Think;
    ent->flashlight->nextthink = level.time + 0.1;
}

void
FL_Think(edict_t *ent)
{
    vec3_t start, end, endp, offset;
    vec3_t fwd, right, up;
    trace_t tr;

    AngleVectors(ent->owner->client->v_angle, fwd, right, up);

    VectorSet(offset, 24, 6, ent->owner->viewheight - 7);
    G_ProjectSource(ent->owner->s.origin, offset, fwd, right, start);
    VectorMA(start, 8, fwd, end);

    tr = gi.trace(start, NULL, NULL, end, ent->owner, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

    if (tr.fraction != 1)
    {
        VectorMA(tr.endpos, -4, fwd, endp);
        VectorCopy(endp, tr.endpos);
    }

    vectoangles(tr.plane.normal, ent->s.angles);
    VectorCopy(tr.endpos, ent->s.origin);

    gi.linkentity(ent);
    ent->nextthink = level.time + 0.1;
}

void 
Use_Battery(edict_t *ent, gitem_t *inv)
{
    int i;

    if (!ent)
    {
        return;
    }

    i = ITEM_INDEX(inv);

    if (!ent->client->pers.inventory[i])
    {
        return;
    }

    FL_Toggle(ent);
}