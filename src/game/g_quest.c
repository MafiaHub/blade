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
 * Quest system.
 *
 * =======================================================================
 */

#include "header/local.h"

qboolean 
Quest_Setstage(int questid, int stage, int depends, edict_t *activator, char *target, char *questname, char *message)
{
	int i, st;
	edict_t *ent, *t;
	int quest_cc = 0;

	if (!activator)
	{
		return false;
	}

	st = game.quest_stage[questid];

	if (game.quest_active == MAX_ACTIVE_QUESTS && stage == QUEST_STAGE_STARTED)
	{
		return false;
	}

	if (st != depends && depends != 0)
	{
		return false;
	}
	else if (game.quest_stage[questid] < stage)
	{
		quest_cc++;
		if (game.quest_stage[questid] == QUEST_STAGE_INACTIVE)
		{
			game.quest_active++;
		}

		game.quest_stage[questid] = stage;

		if (game.quest_stage[questid] >= QUEST_STAGE_COMPLETE)
		{
			game.quest_active--;
		}

		if (message)
		{
			for (i = 0; i < maxclients->value; i++)
			{
				ent = g_edicts + i + 1;

				if (!ent->inuse)
				{
					continue;
				}

				gi.centerprintf(ent, "%s", message);

				QuestsMessage(ent);
				gi.unicast(ent, true);
			}

			Q_strlcpy(game.quest_help[questid][stage], message, strlen(message) + 1);
		}

		if (questname)
		{
			Q_strlcpy(game.quest_titles[questid], questname, strlen(questname) + 1);
		}
		else if (stage == QUEST_STAGE_STARTED)
		{
			gi.dprintf("Unnamed quest ID: %d, FIX THIS!", questid);
			Com_sprintf(game.quest_titles[questid], MAX_QUEST_LENGTH, "Unnamed Quest ID: %d", questid);
		}

		if (stage < QUEST_STAGE_COMPLETE)
		{
			if (game.quest_selected == 0)
			{
				game.quest_selected = questid;
			}
		}
		else
		{
			int j;

			for (j = 0; j < MAX_QUESTS; j++)
			{
				if (game.quest_stage[j] > 0 && game.quest_stage[j] < QUEST_STAGE_COMPLETE)
				{
					game.quest_selected = j;
					break;
				}
			}

			if (j == MAX_QUESTS)
			{
				game.quest_selected = 0;
			}
		}
	}
/* 
		for (i = 0; i < maxclients->value; i++)
		{
			ent = g_edicts + i + 1;

			if (!ent->inuse)
			{
				continue;
			}
		} */

		if (target && quest_cc)
		{
			t = NULL;

			while ((t = G_Find(t, FOFS(targetname), target)))
			{
				if (t->use)
				{
					t->use(t, activator, activator);
				}
			}
		}

		return quest_cc != 0;
}

/*
 * QUAKED quest_setstage 
 * Sets stage of a quest to a specific value.
 * Value of 100 means quest was successfully finished,
 * value of 150 signifies quest failure.
 * 
 */
void
quest_setstage_use(edict_t *self, edict_t *other /* unused */, edict_t *activator)
{
	qboolean q = Quest_Setstage(self->questid, self->stage, self->depends, activator, self->target, self->questname, self->message);

	if (self->killtarget && q)
	{
		edict_t *t = NULL;

		while ((t = G_Find(t, FOFS(targetname), self->killtarget)))
		{
			/* decrement secret count if target_secret is removed */
			if (!Q_stricmp(t->classname, "target_secret"))
			{
				level.total_secrets--;
			}
			/* same deal with target_goal, but also turn off CD music if applicable */
			else if (!Q_stricmp(t->classname, "target_goal"))
			{
				level.total_goals--;

				if (level.found_goals >= level.total_goals)
				{
					gi.configstring(CS_CDTRACK, "0");
				}
			}

			G_FreeEdict(t);

			if (!self->inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}
}

void
SP_quest_setstage(edict_t *self)
{
	if (!self)
	{
		return;
	}

    if (deathmatch->value)
    {
        return;
    }

    self->use = quest_setstage_use;
}

/* ===================================================== */
