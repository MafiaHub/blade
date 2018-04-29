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
FL_Toggle(edict_t *self)
{
    vec3_t start, fwd, end, right;

    if (self->flashlight)
    {
        G_FreeEdict(self->flashlight);
        self->flashlight = NULL;
        return;
    }

    AngleVectors(self->client->v_angle, fwd, right, NULL);

    VectorSet(end, 100, 0, 0);
    G_ProjectSource(self->s.origin, end, fwd, right, start);
    self->flashlight = G_Spawn();
    self->flashlight->owner = self;
    self->flashlight->movetype = MOVETYPE_NOCLIP;
    self->flashlight->solid = SOLID_NOT;
    self->flashlight->classname = "flashlight";
    self->flashlight->s.modelindex = gi.modelindex("sprites/s_empty.sp2");
    self->flashlight->s.skinnum = 0;
    self->flashlight->s.effects |= EF_FLASHLIGHT;
    self->flashlight->think = FL_Think;
    self->flashlight->nextthink = level.time + 0.1;
}

void
FL_Think(edict_t *self)
{
    vec3_t start, end, endp, offset;
    vec3_t fwd, right, up;
    trace_t tr;

    AngleVectors(self->owner->client->v_angle, fwd, right, up);

    VectorSet(offset, 24, 6, self->owner->viewheight - 7);
    G_ProjectSource(self->owner->s.origin, offset, fwd, right, start);
    VectorMA(start, 8, fwd, end);

    tr = gi.trace(start, NULL, NULL, end, self->owner, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

    if (tr.fraction != 1)
    {
        VectorMA(tr.endpos, -4, fwd, endp);
        VectorCopy(endp, tr.endpos);
    }

    vectoangles(tr.plane.normal, self->s.angles);
    VectorCopy(tr.endpos, self->s.origin);

    gi.linkentity(self);
    self->nextthink = level.time + 0.1;
}