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
 * Item spawning.
 *
 * =======================================================================
 */

#include "header/local.h"

typedef struct
{
	char *name;
	void (*spawn)(edict_t *ent);
} spawn_t;

void SP_item_health(edict_t *self);
void SP_item_health_small(edict_t *self);
void SP_item_health_large(edict_t *self);
void SP_item_health_mega(edict_t *self);

void SP_info_player_start(edict_t *ent);
void SP_info_player_deathmatch(edict_t *ent);
void SP_info_player_coop(edict_t *ent);
void SP_info_player_intermission(edict_t *ent);

void SP_func_plat(edict_t *ent);
void SP_func_rotating(edict_t *ent);
void SP_func_button(edict_t *ent);
void SP_func_door(edict_t *ent);
void SP_func_door_secret(edict_t *ent);
void SP_func_door_rotating(edict_t *ent);
void SP_func_water(edict_t *ent);
void SP_func_train(edict_t *ent);
void SP_func_conveyor(edict_t *self);
void SP_func_wall(edict_t *self);
void SP_func_object(edict_t *self);
void SP_func_explosive(edict_t *self);
void SP_func_timer(edict_t *self);
void SP_func_mirror(edict_t *ent);
void SP_func_areaportal(edict_t *ent);
void SP_func_clock(edict_t *ent);
void SP_func_killbox(edict_t *ent);

void SP_trigger_always(edict_t *ent);
void SP_trigger_once(edict_t *ent);
void SP_trigger_multiple(edict_t *ent);
void SP_trigger_relay(edict_t *ent);
void SP_trigger_push(edict_t *ent);
void SP_trigger_hurt(edict_t *ent);
void SP_trigger_key(edict_t *ent);
void SP_trigger_counter(edict_t *ent);
void SP_trigger_elevator(edict_t *ent);
void SP_trigger_gravity(edict_t *ent);
void SP_trigger_monsterjump(edict_t *ent);

void SP_target_actor (edict_t *ent);
void SP_target_temp_entity(edict_t *ent);
void SP_target_speaker(edict_t *ent);
void SP_target_explosion(edict_t *ent);
void SP_target_changelevel(edict_t *ent);
void SP_target_secret(edict_t *ent);
void SP_target_goal(edict_t *ent);
void SP_target_splash(edict_t *ent);
void SP_target_spawner(edict_t *ent);
void SP_target_blaster(edict_t *ent);
void SP_target_crosslevel_trigger(edict_t *ent);
void SP_target_crosslevel_target(edict_t *ent);
void SP_target_laser(edict_t *self);
void SP_target_help(edict_t *ent);
void SP_target_lightramp(edict_t *self);
void SP_target_earthquake(edict_t *ent);
void SP_target_character(edict_t *ent);
void SP_target_string(edict_t *ent);

void SP_worldspawn(edict_t *ent);
void SP_viewthing(edict_t *ent);

void SP_light(edict_t *self);
void SP_light_mine1(edict_t *ent);
void SP_light_mine2(edict_t *ent);
void SP_info_null(edict_t *self);
void SP_info_notnull(edict_t *self);
void SP_path_corner(edict_t *self);
void SP_point_combat(edict_t *self);
void SP_ai_node(edict_t *self);

void SP_actor_bartender(edict_t *self);

void SP_misc_actor(edict_t *self);
void SP_misc_explobox(edict_t *self);
void SP_misc_banner(edict_t *self);
void SP_misc_satellite_dish(edict_t *self);
void SP_misc_gib_arm(edict_t *self);
void SP_misc_gib_leg(edict_t *self);
void SP_misc_gib_head(edict_t *self);
void SP_misc_deadsoldier(edict_t *self);
void SP_misc_viper(edict_t *self);
void SP_misc_viper_bomb(edict_t *self);
void SP_misc_bigviper(edict_t *self);
void SP_misc_strogg_ship(edict_t *self);
void SP_misc_teleporter(edict_t *self);
void SP_misc_teleporter_dest(edict_t *self);
void SP_misc_blackhole(edict_t *self);
void SP_misc_mesh(edict_t *self);

void SP_monster_commander_body(edict_t *self);

void SP_quest_setstage(edict_t *self);

spawn_t spawns[] = {
	#include "g_spawnlist.c"

	{NULL, NULL}
};

void
mesh_think(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->s.frame++;

	if (ent->s.frame < ent->maxframes)
	{
		ent->nextthink = level.time + FRAMETIME;
	}
	else if (ent->animloop)
	{
		ent->s.frame = ent->minframes;
		ent->nextthink = level.time + FRAMETIME;
	}
}

void
mesh_use(edict_t *ent, edict_t *other /* unused */, edict_t *activator /* unused */)
{
	if (!ent)
	{
		return;
	}

	if (ent->think)
	{
		ent->think = NULL;
		ent->nextthink = 99999999;
		ent->s.frame = ent->minframes;
	}
	else
	{
		ent->think = mesh_think;
		ent->nextthink = level.time + FRAMETIME;
	}
}

/*
 * Finds the spawn function for
 * the entity and calls it
 */
void
ED_CallSpawn(edict_t *ent)
{
	spawn_t *s;
	gitem_t *item;
	int i;

	if (!ent)
	{
		return;
	}

	if (!ent->classname)
	{
		gi.dprintf("ED_CallSpawn: NULL classname\n");
		G_FreeEdict(ent);
		return;
	}

	/* check item spawn functions */
	for (i = 0, item = itemlist; i < game.num_items; i++, item++)
	{
		if (!item->classname)
		{
			continue;
		}

		if (!strcmp(item->classname, ent->classname))
		{
			/* found it */
			SpawnItem(ent, item);
			return;
		}
	}

	/* check normal spawn functions */
	for (s = spawns; s->name; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{
			/* found it */
			s->spawn(ent);
			return;
		}
	}

	/* check for static mesh spawn */
	if (!strncmp("mesh_", ent->classname, 5))
	{
		char modelpath[MAX_QPATH] = {0};
		sprintf(modelpath, "models/objects/%s/tris.md2", ent->classname + 5);

		ent->movetype = MOVETYPE_NONE;
		ent->solid = SOLID_BBOX;
		ent->s.modelindex = gi.modelindex(modelpath);
		ent->s.frame = ent->minframes;

		if (ent->maxframes && !ent->targetname)
		{
			ent->think = mesh_think;
			ent->nextthink = level.time + FRAMETIME;
		}
		
		if (ent->targetname)
		{
			ent->use = mesh_use;
		}

		gi.linkentity(ent);
		return;
	}

	gi.dprintf("%s doesn't have a spawn function\n", ent->classname);
}

char *
ED_NewString(const char *string)
{
	char *newb, *new_p;
	int i, l;

	if (!string)
	{
		return NULL;
	}

	l = strlen(string) + 1;

	newb = gi.TagMalloc(l, TAG_LEVEL);

	new_p = newb;

	for (i = 0; i < l; i++)
	{
		if ((string[i] == '\\') && (i < l - 1))
		{
			i++;

			if (string[i] == 'n')
			{
				*new_p++ = '\n';
			}
			else
			{
				*new_p++ = '\\';
			}
		}
		else
		{
			*new_p++ = string[i];
		}
	}

	return newb;
}

/*
 * Takes a key/value pair and sets
 * the binary values in an edict
 */
void
ED_ParseField(const char *key, const char *value, edict_t *ent)
{
	field_t *f;
	byte *b;
	float v;
	vec3_t vec;

	if (!key || !value)
	{
		return;
	}

	for (f = fields; f->name; f++)
	{
		if (!(f->flags & FFL_NOSPAWN) && !Q_strcasecmp(f->name, (char *)key))
		{
			/* found it */
			if (f->flags & FFL_SPAWNTEMP)
			{
				b = (byte *)&st;
			}
			else
			{
				b = (byte *)ent;
			}

			switch (f->type)
			{
				case F_LSTRING:
					*(char **)(b + f->ofs) = ED_NewString(value);
					break;
				case F_VECTOR:
					sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
					((float *)(b + f->ofs))[0] = vec[0];
					((float *)(b + f->ofs))[1] = vec[1];
					((float *)(b + f->ofs))[2] = vec[2];
					break;
				case F_INT:
					*(int *)(b + f->ofs) = (int)strtol(value, (char **)NULL, 10);
					break;
				case F_FLOAT:
					*(float *)(b + f->ofs) = (float)strtod(value, (char **)NULL);
					break;
				case F_ANGLEHACK:
					v = (float)strtod(value, (char **)NULL);
					((float *)(b + f->ofs))[0] = 0;
					((float *)(b + f->ofs))[1] = v;
					((float *)(b + f->ofs))[2] = 0;
					break;
				case F_IGNORE:
					break;
				default:
					break;
			}

			return;
		}
	}
}

/*
 * Parses an edict out of the given string,
 * returning the new position ed should be
 * a properly initialized empty edict.
 */
char *
ED_ParseEdict(char *data, edict_t *ent)
{
	qboolean init;
	char keyname[256];
	const char *com_token;

	if (!ent)
	{
		return NULL;
	}

	init = false;
	memset(&st, 0, sizeof(st));

	/* go through all the dictionary pairs */
	while (1)
	{
		/* parse key */
		com_token = COM_Parse(&data);

		if (com_token[0] == '}')
		{
			break;
		}

		if (!data)
		{
			gi.error("ED_ParseEntity: EOF without closing brace");
		}

		Q_strlcpy(keyname, com_token, sizeof(keyname));

		/* parse value */
		com_token = COM_Parse(&data);

		if (!data)
		{
			gi.error("ED_ParseEntity: EOF without closing brace");
		}

		if (com_token[0] == '}')
		{
			gi.error("ED_ParseEntity: closing brace without data");
		}

		init = true;

		ED_ParseField(keyname, com_token, ent);
	}

	if (!init)
	{
		memset(ent, 0, sizeof(*ent));
	}

	return data;
}

/*
 * Chain together all entities with a matching team field.
 *
 * All but the first will have the FL_TEAMSLAVE flag set.
 * All but the last will have the teamchain field set to the next one
 */
void
G_FindTeams(void)
{
	edict_t *e, *e2, *chain;
	int i, j;
	int c, c2;

	c = 0;
	c2 = 0;

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse)
		{
			continue;
		}

		if (!e->team)
		{
			continue;
		}

		if (e->flags & FL_TEAMSLAVE)
		{
			continue;
		}

		chain = e;
		e->teammaster = e;
		c++;
		c2++;

		for (j = i + 1, e2 = e + 1; j < globals.num_edicts; j++, e2++)
		{
			if (!e2->inuse)
			{
				continue;
			}

			if (!e2->team)
			{
				continue;
			}

			if (e2->flags & FL_TEAMSLAVE)
			{
				continue;
			}

			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	gi.dprintf("%i teams with %i entities.\n", c, c2);
}

/*
 * Creates a server's entity / program execution context by
 * parsing textual entity definitions out of an ent file.
 */
void
SpawnEntities(const char *mapname, char *entities, const char *spawnpoint)
{
	edict_t *ent;
	int inhibit;
	const char *com_token;
	int i;
	float skill_level;
	static qboolean monster_count_city3 = false;

	if (!mapname || !entities || !spawnpoint)
	{
		return;
	}

	skill_level = floor(skill->value);

	if (skill_level < 0)
	{
		skill_level = 0;
	}

	if (skill_level > 3)
	{
		skill_level = 3;
	}

	if (skill->value != skill_level)
	{
		gi.cvar_forceset("skill", va("%f", skill_level));
	}

	SaveClientData();

	gi.FreeTags(TAG_LEVEL);

	memset(&level, 0, sizeof(level));
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

	Q_strlcpy(level.mapname, mapname, sizeof(level.mapname));
	Q_strlcpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint));

	/* set client fields on player ents */
	for (i = 0; i < game.maxclients; i++)
	{
		g_edicts[i + 1].client = game.clients + i;
	}

	ent = NULL;
	inhibit = 0;

	/* parse ents */
	while (1)
	{
		/* parse the opening brace */
		com_token = COM_Parse(&entities);

		if (!entities)
		{
			break;
		}

		if (com_token[0] != '{')
		{
			gi.error("ED_LoadFromFile: found %s when expecting {", com_token);
		}

		if (!ent)
		{
			ent = g_edicts;
		}
		else
		{
			ent = G_Spawn();
		}

		entities = ED_ParseEdict(entities, ent);

		/* yet another map hack */
		if (!Q_stricmp(level.mapname, "command") &&
			!Q_stricmp(ent->classname, "trigger_once") &&
		   	!Q_stricmp(ent->model, "*27"))
		{
			ent->spawnflags &= ~SPAWNFLAG_NOT_HARD;
		}

		/*
		 * The 'monsters' count in city3.bsp is wrong.
		 * There're two monsters triggered in a hidden
		 * and unreachable room next to the security
		 * pass.
		 *
		 * We need to make sure that this hack is only
		 * applied once!
		 */
		if(!Q_stricmp(level.mapname, "city3") && !monster_count_city3)
		{
			level.total_monsters = level.total_monsters - 2;
			monster_count_city3 = true;
		}

		/* remove things (except the world) from
		   different skill levels or deathmatch */
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
				{
					G_FreeEdict(ent);
					inhibit++;
					continue;
				}
			}
			else
			{
				if (((skill->value == 0) &&
					 (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) &&
					 (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(((skill->value == 2) ||
					  (skill->value == 3)) &&
					 (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
				{
					G_FreeEdict(ent);
					inhibit++;
					continue;
				}
			}

			ent->spawnflags &=
				~(SPAWNFLAG_NOT_EASY | SPAWNFLAG_NOT_MEDIUM |
				  SPAWNFLAG_NOT_HARD |
				  SPAWNFLAG_NOT_COOP | SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn(ent);
	}

	gi.dprintf("%i entities inhibited.\n", inhibit);

	G_FindTeams();

	PlayerTrail_Init();
	GenerateNavigationGrid();
}

/* =================================================================== */

char *single_statusbar =
	"yb	-44 "

/* health */
	"xl	50 "
	"pic 0 "
	"xl	74 "
	"hnum "

/* ammo */
	"if 2 "
	"	xr -74 "
	"   pic 2 "
	"   xr -124 "
	"   anum "
	"   yb -44 "
	"endif "

/* magazine bullet count */
	/*"if 18 "
	"   xr -124 "
	"   mnum "
	"endif " */

/* armor */
	"if 4 "
	"	xl	50 "
	"   yb -70 "
	"	pic 4 "
	"	xl	74 "
	"	rnum "
	"endif "

/* hotbar */
	"xl 0 "
	"yb -44 "
	"hotb "

/* selected item */
/* 	"if 6 "
	"	xr -74 "
	"   yb -70 "
	"	pic 6 "
	"endif "

	"yb	-70 " */

/* picked up item */
	"if 7 "
	"	xv	0 "
	"	pic 7 "
	"	xv	26 "
	"	yb	-62 "
	"	stat_string 8 "
	"	yb	-70 "
	"endif "

/* timer */
	"if 9 "
	"	xv	262 "
	"	num	2	10 "
	"	xv	296 "
	"	pic	9 "
	"endif "

/*  help / weapon icon */
	/* "if 11 "
	"	xv	340 "
	"	pic	11 "
	"endif " */
;

char *dm_statusbar =
	"yb	-44 "

/* health */
	"xl	50 "
	"pic 0 "
	"xl	74 "
	"hnum "

/* ammo */
	"if 2 "
	"	xr -74 "
	"   pic 2 "
	"   xr -124 "
	"   anum "
	"   yb -44 "
	"endif "

/* magazine bullet count */
	/*"if 18 "
	"   xr -124 "
	"   mnum "
	"endif " */

/* armor */
	"if 4 "
	"	xl	50 "
	"   yb -70 "
	"	pic 4 "
	"	xl	74 "
	"	rnum "
	"endif "

/* selected item */
/* 	"if 6 "
	"	xr -74 "
	"   yb -70 "
	"	pic 6 "
	"endif "

	"yb	-70 " */

/* picked up item */
	"if 7 "
	"	xv	0 "
	"	yb	-62 "
	"	pic 7 "
	"	yb	-62 "
	"	xv	26 "
	"	stat_string 8 "
	"	yb	-70 "
	"endif "

/* timer */
	"if 9 "
	"	xv	262 "
	"	num	2	10 "
	"	xv	296 "
	"	pic	9 "
	"endif "

/*  help / weapon icon */
/* 	"if 11 "
	"	xv	148 "
	"	pic	11 "
	"endif "
 */
/*  frags */
	"xr	-50 "
	"yt 2 "
	"num 3 14 "

/* spectator */
	"if 17 "
	"xv 0 "
	"yb -58 "
	"string2 \"SPECTATOR MODE\" "
	"endif "

/* chase camera */
	"if 16 "
	"xv 0 "
	"yb -68 "
	"string \"Chasing\" "
	"xv 64 "
	"stat_string 16 "
	"endif "
;

/*QUAKED worldspawn (0 0 0) ?
 *
 * Only used for the world.
 *  "sky"		environment map name
 *  "skyaxis"	vector axis for rotating sky
 *  "skyrotate"	speed of rotation in degrees/second
 *  "sounds"	music cd track number
 *  "gravity"	800 is default gravity
 *  "message"	text to print at user logon
 */
void
SP_worldspawn(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true; /* since the world doesn't use G_Spawn() */
	ent->s.modelindex = 1; /* world model is always index 1 */

	/* --------------- */

	/* reserve some spots for dead
	   player bodies for coop / deathmatch */
	InitBodyQue();

	/* set configstrings for items */
	SetItemNames();

	if (st.nextmap)
	{
		strcpy(level.nextmap, st.nextmap);
	}

	/* make some data visible to the server */
	if (ent->message && ent->message[0])
	{
		gi.configstring(CS_NAME, ent->message);
		Q_strlcpy(level.level_name, ent->message, sizeof(level.level_name));
	}
	else
	{
		Q_strlcpy(level.level_name, level.mapname, sizeof(level.level_name));
	}

	if (st.sky && st.sky[0])
	{
		gi.configstring(CS_SKY, st.sky);
	}
	else
	{
		gi.configstring(CS_SKY, "unit1_");
	}

	gi.configstring(CS_SKYROTATE, va("%f", st.skyrotate));

	gi.configstring(CS_SKYAXIS, va("%f %f %f",
				st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]));

	gi.configstring(CS_FOGDENSITY, va("%f", st.fogdensity));
	gi.configstring(CS_FOGCOLOR, va("%f %f %f", st.fogcolor[0], st.fogcolor[1], st.fogcolor[2]));

	gi.configstring(CS_CDTRACK, va("%i", ent->sounds));

	gi.configstring(CS_MAXCLIENTS, va("%i", (int)(maxclients->value)));

	/* status bar program */
	if (deathmatch->value)
	{
		gi.configstring(CS_STATUSBAR, dm_statusbar);
	}
	else
	{
		gi.configstring(CS_STATUSBAR, single_statusbar);
	}

	/* --------------- */

	/* help icon for statusbar */
	gi.imageindex("i_help");
	level.pic_health = gi.imageindex("i_health");
	gi.imageindex("help");
	gi.imageindex("field_3");

	if (!st.gravity)
	{
		gi.cvar_set("sv_gravity", "800");
	}
	else
	{
		gi.cvar_set("sv_gravity", st.gravity);
	}

	snd_fry = gi.soundindex("player/fry.wav"); /* standing in lava / slime */

	gi.soundindex("player/lava1.wav");
	gi.soundindex("player/lava2.wav");

	gi.soundindex("misc/pc_up.wav");
	gi.soundindex("misc/talk1.wav");

	gi.soundindex("misc/udeath.wav");

	/* gibs */
	gi.soundindex("items/respawn1.wav");

	/* sexed sounds */
	gi.soundindex("*death1.wav");
	gi.soundindex("*death2.wav");
	gi.soundindex("*death3.wav");
	gi.soundindex("*death4.wav");
	gi.soundindex("*fall1.wav");
	gi.soundindex("*fall2.wav");
	gi.soundindex("*gurp1.wav"); /* drowning damage */
	gi.soundindex("*gurp2.wav");
	gi.soundindex("*jump1.wav"); /* player jump */
	gi.soundindex("*pain25_1.wav");
	gi.soundindex("*pain25_2.wav");
	gi.soundindex("*pain50_1.wav");
	gi.soundindex("*pain50_2.wav");
	gi.soundindex("*pain75_1.wav");
	gi.soundindex("*pain75_2.wav");
	gi.soundindex("*pain100_1.wav");
	gi.soundindex("*pain100_2.wav");

	/* sexed models: THIS ORDER MUST MATCH THE DEFINES IN g_local.h
	   you can add more, max 19 (pete change)these models are only
	   loaded in coop or deathmatch. not singleplayer. */
	if (coop->value || deathmatch->value)
	{
		gi.modelindex("#w_blaster.md2");
		gi.modelindex("#w_shotgun.md2");
		gi.modelindex("#w_sshotgun.md2");
		gi.modelindex("#w_machinegun.md2");
		gi.modelindex("#w_chaingun.md2");
		gi.modelindex("#a_grenades.md2");
		gi.modelindex("#w_glauncher.md2");
		gi.modelindex("#w_rlauncher.md2");
		gi.modelindex("#w_hyperblaster.md2");
		gi.modelindex("#w_railgun.md2");
		gi.modelindex("#w_bfg.md2");
	}

	/* ------------------- */

	gi.soundindex("player/gasp1.wav"); /* gasping for air */
	gi.soundindex("player/gasp2.wav"); /* head breaking surface, not gasping */

	gi.soundindex("player/watr_in.wav"); /* feet hitting water */
	gi.soundindex("player/watr_out.wav"); /* feet leaving water */

	gi.soundindex("player/watr_un.wav"); /* head going underwater */

	gi.soundindex("player/u_breath1.wav");
	gi.soundindex("player/u_breath2.wav");

	gi.soundindex("items/pkup.wav"); /* bonus item pickup */
	gi.soundindex("world/land.wav"); /* landing thud */
	gi.soundindex("misc/h2ohit1.wav"); /* landing splash */

	gi.soundindex("items/damage.wav");
	gi.soundindex("items/protect.wav");
	gi.soundindex("items/protect4.wav");
	gi.soundindex("weapons/noammo.wav");

	gi.soundindex("infantry/inflies1.wav");

	sm_meat_index = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex("models/objects/gibs/arm/tris.md2");
	gi.modelindex("models/objects/gibs/bone/tris.md2");
	gi.modelindex("models/objects/gibs/bone2/tris.md2");
	gi.modelindex("models/objects/gibs/chest/tris.md2");
	gi.modelindex("models/objects/gibs/skull/tris.md2");
	gi.modelindex("models/objects/gibs/head2/tris.md2");

	/* Setup light animation tables. 'a'
	   is total darkness, 'z' is doublebright. */

	/* 0 normal */
	gi.configstring(CS_LIGHTS + 0, "m");

	/* 1 FLICKER (first variety) */
	gi.configstring(CS_LIGHTS + 1, "mmnmmommommnonmmonqnmmo");

	/* 2 SLOW STRONG PULSE */
	gi.configstring(CS_LIGHTS + 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	/* 3 CANDLE (first variety) */
	gi.configstring(CS_LIGHTS + 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	/* 4 FAST STROBE */
	gi.configstring(CS_LIGHTS + 4, "mamamamamama");

	/* 5 GENTLE PULSE 1 */
	gi.configstring(CS_LIGHTS + 5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	/* 6 FLICKER (second variety) */
	gi.configstring(CS_LIGHTS + 6, "nmonqnmomnmomomno");

	/* 7 CANDLE (second variety) */
	gi.configstring(CS_LIGHTS + 7, "mmmaaaabcdefgmmmmaaaammmaamm");

	/* 8 CANDLE (third variety) */
	gi.configstring(CS_LIGHTS + 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	/* 9 SLOW STROBE (fourth variety) */
	gi.configstring(CS_LIGHTS + 9, "aaaaaaaazzzzzzzz");

	/* 10 FLUORESCENT FLICKER */
	gi.configstring(CS_LIGHTS + 10, "mmamammmmammamamaaamammma");

	/* 11 SLOW PULSE NOT FADE TO BLACK */
	gi.configstring(CS_LIGHTS + 11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	/* styles 32-62 are assigned by the light program for switchable lights */

	/* 63 testing */
	gi.configstring(CS_LIGHTS + 63, "a");
}
