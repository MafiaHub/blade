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
 * Bartender actor
 *
 * =======================================================================
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"

#include "../../header/local.h"
#include "../misc/actor.h"

static mframe_t actor_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};

static mmove_t actor_move_stand = {FRAME_stand101, FRAME_stand140, actor_frames_stand, NULL};

static void actor_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &actor_move_stand;

	// randomize on startup
	if (level.time < 1.0)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));
}

void bartender_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self || !other || !other->client)
	{
		return;
	}

	gi.cprintf(other, PRINT_HIGH, "\"It just works!\" - Todd Howard\n");

	/* 
	VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
	self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
	self->monsterinfo.walk (self);
	self->target = NULL; */
}


/*QUAKED actor_bartender (1 .5 0) (-16 -16 -24) (16 16 32)
*/

void SP_actor_bartender (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (!self->targetname)
	{
		gi.dprintf("untargeted %s at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/objects/talking_guy/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	if (!self->health)
		self->health = 100;
	self->mass = 200;

	self->pain = NULL;
	self->die = NULL;

	self->monsterinfo.stand = actor_stand;
	self->monsterinfo.walk = NULL;
	self->monsterinfo.run = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &actor_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);

	self->use = bartender_use;
}

#pragma GCC diagnostic pop
