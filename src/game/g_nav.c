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
 * Waypoint and targeting system for actors
 *
 * =======================================================================
 */

#include "header/local.h"

/*QUAKED target_actor (.5 .3 0) (-8 -8 -8) (8 8 8) JUMP SHOOT ATTACK x HOLD BRUTAL
JUMP			jump in set direction upon reaching this target
SHOOT			take a single shot at the pathtarget
ATTACK			attack pathtarget until it or actor is dead 

"target"		next target_actor
"pathtarget"	target of any action to be taken at this point
"wait"			amount of time actor should pause at this point
"message"		actor will "say" this to the player

for JUMP only:
"speed"			speed thrown forward (default 200)
"height"		speed thrown upwards (default 200)
*/

void target_actor_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	v;

	if (other->movetarget != self)
		return;
	
	if (other->enemy)
		return;

	other->goalentity = other->movetarget = NULL;

	if (self->spawnflags & 1)		//jump
	{
		other->velocity[0] = self->movedir[0] * self->speed;
		other->velocity[1] = self->movedir[1] * self->speed;
		
		if (other->groundentity)
		{
			other->groundentity = NULL;
			other->velocity[2] = self->movedir[2];
			gi.sound(other, CHAN_VOICE, gi.soundindex("player/male/jump1.wav"), 1, ATTN_NORM, 0);
		}
	}

	if (self->spawnflags & 2)	//shoot
	{
	}
	else if (self->spawnflags & 4)	//attack
	{
		other->enemy = G_PickTarget(self->pathtarget);
		if (other->enemy)
		{
			other->goalentity = other->enemy;
			if (self->spawnflags & 32)
				other->monsterinfo.aiflags |= AI_BRUTAL;
			if (self->spawnflags & 16)
			{
				other->monsterinfo.aiflags |= AI_STAND_GROUND;
				other->monsterinfo.stand(other);
			}
			else
			{
				other->monsterinfo.run(other);
			}
		}
	}

	if (!(self->spawnflags & 6) && (self->pathtarget))
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}

	other->movetarget = G_PickTarget(self->target);

	if (!other->goalentity)
		other->goalentity = other->movetarget;

	if (!other->movetarget && !other->enemy)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand (other);
	}
	else if (other->movetarget == other->goalentity)
	{
		VectorSubtract (other->movetarget->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw (v);
	}
}

void SP_target_actor (edict_t *self)
{
	if (!self->targetname)
		gi.dprintf ("%s with no targetname at %s\n", self->classname, vtos(self->s.origin));

	self->solid = SOLID_TRIGGER;
	self->touch = target_actor_touch;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->svflags = SVF_NOCLIENT;

	if (self->spawnflags & 1)
	{
		if (!self->speed)
			self->speed = 200;
		if (!st.height)
			st.height = 200;
		if (self->s.angles[YAW] == 0)
			self->s.angles[YAW] = 360;
		G_SetMovedir (self->s.angles, self->movedir);
		self->movedir[2] = st.height;
	}

	gi.linkentity (self);
}

static edict_t *s_rootNode;

edict_t*
FindClosestNode(vec3_t target)
{
	int i;
	edict_t *e, *closest_node;
	trace_t tr;
	float dist, smallest_dist=999999;
	vec3_t v;

	e = &g_edicts[(int)maxclients->value + 1];
	closest_node = NULL;

	for (i = maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse || (strcmp(e->classname, "ai_node") != 0))
		{
			continue;
		}

		tr = gi.trace(e->s.origin, NULL, NULL, target, e, CONTENTS_SOLID);

		if (tr.fraction != 1 && (strcmp(tr.ent->classname, "ai_node") != 0))
		{
			continue;
		}
		
		VectorSubtract(e->s.origin, target, v);
		dist = VectorLength(v);

		if (dist < smallest_dist)
		{
			smallest_dist = dist;
			closest_node = e;
		}
	}

	if (closest_node == NULL)
	{
		gi.dprintf("Navigation node wasn't found for target!\n");
	}

	return closest_node;
}

void
ClearNavlinkStates()
{
	int i;
	edict_t *e;
	e = &g_edicts[(int)maxclients->value + 1];

	for (i = maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse || (strcmp(e->classname, "ai_node") != 0))
		{
			continue;
		}

		e->was_swept = false;

		e->s.skinnum = 0;
		gi.linkentity(e);
	}

	s_rootNode = NULL;
}

edict_t*
PickClosestNode(edict_t* ent, vec3_t target)
{
	edict_t *node=0, *endnode=0, *viable_node=0;
	int found=0, smallest_dist=99999999;

	if (!ent)
	{
		return NULL;
	}

	ClearNavlinkStates();
	node = FindClosestNode(ent->s.origin);
	endnode = FindClosestNode(target);

	if (!node || !endnode)
	{
		return NULL;
	}

	return NULL;
}

int
IsNodeReachable(edict_t *node, edict_t *other)
{
	vec3_t start, end;
	trace_t tr;

	if (!node || !other)
	{
		return false;
	}

	VectorCopy(node->s.origin, start);
	VectorCopy(other->s.origin, end);

	start[2] += 2;
	end[2] += 2;

	tr = gi.trace(start, NULL, NULL, end, node, CONTENTS_SOLID);

	if (tr.fraction == 1)
	{
		return true;
	}

	if (strcmp(tr.ent->classname, "ai_node") == 0)
	{
		return true;
	}

	return false;
}

void
FindNavlinksForNode(edict_t *node, int debug_index)
{
	int i;
	edict_t *e;

	if (!node)
	{
		return;
	}

	e = &g_edicts[(int)maxclients->value + 1];

	for (i = maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse || (strcmp(e->classname, "ai_node") != 0))
		{
			continue;
		}

		if (node->num_navlinks >= MAX_NAV_LINKS)
		{
			return;
		}

		if (e == node)
		{
			continue;
		}

		if (IsNodeReachable(node, e))
		{
			node->navlinks[node->num_navlinks] = e;
			node->num_navlinks++;

			gi.dprintf("Navigation Node: %d found for node: %d\n", i, debug_index);
		}
	}
}

void
GenerateNavigationGrid()
{
	int i;
	edict_t *e;
	gi.dprintf("Building navmesh...\n");

	e = &g_edicts[(int)maxclients->value + 1];

	for (i = maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse || (strcmp(e->classname, "ai_node") != 0))
		{
			continue;
		}

		FindNavlinksForNode(e, i);
	}
}

void ai_node_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!self || !other)
	{
		return;
	}

	if (other->followentity != self)
	{
		return;
	}
}

void SP_ai_node(edict_t *self)
{
	self->solid = SOLID_TRIGGER;
	self->touch = ai_node_touch;
	self->s.skinnum = 0;
	self->s.modelindex = gi.modelindex("models/dev/node/tris.md2");
	self->inuse = true;
	self->svflags = SVF_NOCLIENT;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	if (self->spawnflags & 1)
	{
		if (!self->speed)
			self->speed = 200;
		if (!st.height)
			st.height = 200;
		if (self->s.angles[YAW] == 0)
			self->s.angles[YAW] = 360;
		G_SetMovedir (self->s.angles, self->movedir);
		self->movedir[2] = st.height;
	}

	gi.linkentity(self);
}