/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Player
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Player management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#include <limits.h>

#include <stdlib.h>

#include <ARX_Menu.h>
#include <ARX_Player.h>

#include <HERMESMain.h>

#include <EERIEMath.h>
#include <EERIEObject.h>
#include <EERIEPoly.h>
#include <EERIEDraw.h>
#include <EERIEPathfinder.h>
#include "EERIERenderer.h"

#include "ARX_Changelevel.h"
#include "ARX_Collisions.h"
#include "ARX_CParticles.h"
#include "ARX_Damages.h"
#include "ARX_Equipment.h"
#include "ARX_Fogs.h"
#include "ARX_GlobalMods.h"
#include "ARX_Loc.h"
#include "ARX_Missile.h"
#include "ARX_NPC.h"
#include "ARX_Particles.h"
#include "ARX_Paths.h"
#include "ARX_Text.h"
#include "ARX_Time.h"
#include "ARX_Script.h"
#include "ARX_Sound.h"
#include "ARX_Special.h"
#include "ARX_Speech.h"
#include "ARX_Spells.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
extern long		USE_NEW_SKILLS;
extern long		ARX_CONVERSATION;
extern long		HERO_SHOW_1ST;
extern long		REQUEST_SPEECH_SKIP;
extern long		NEED_DEBUGGER_CLEAR;
extern long		CHANGE_LEVEL_ICON;
extern long		DONT_ERASE_PLAYER;
extern long		GLOBAL_MAGIC_MODE;
extern EERIE_3D	PUSH_PLAYER_FORCE;
extern QUAKE_FX_STRUCT QuakeFx;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern TextureContainer * iconequip[];
extern CParticleManager * pParticleManager;

extern unsigned long LAST_JUMP_ENDTIME;
//-----------------------------------------------------------------------------
#define WORLD_DAMPING	0.35f
#define WORLD_GRAVITY	0.1f
#define JUMP_GRAVITY	0.02f //OLD SETTING 0.03f
#define STEP_DISTANCE	120.f

//-----------------------------------------------------------------------------
extern EERIE_3D PUSH_PLAYER_FORCE;
extern bool bBookHalo;
extern bool bGoldHalo;
extern float InventoryX;
extern float InventoryDir;
extern long APPLY_PUSH;
extern long COLLIDED_CLIMB_POLY;
extern long HERO_SHOW_1ST;
extern long STARTED_A_GAME;
extern long SHOW_TORCH;
extern long TRUE_PLAYER_MOUSELOOK_ON;
extern unsigned long ulBookHaloTime;
extern unsigned long ulGoldHaloTime;
extern long cur_rf;
//-----------------------------------------------------------------------------
static const float ARX_PLAYER_SKILL_STEALTH_MAX(100.0F);

//-----------------------------------------------------------------------------
ARXCHARACTER playerCharacter;
EERIE_3DOBJ * hero = NULL;
float currentdistance = 0.f;
float CURRENT_PLAYER_COLOR = 0;
float Full_Jump_Height = 0;
float DeadCameraDistance = 0.f;
float PLAYER_ROTATION = 0;

long USE_PLAYERCOLLISIONS = 1;
long BLOCK_PLAYER_CONTROLS = 0;
long WILLRETURNTOCOMBATMODE = 0;
long DeadTime = 0;
unsigned long LastHungerSample = 0;
unsigned long ROTATE_START = 0;
long sp_max = 0;
//-----------------------------------------------------------------------------
// Player Anims FLAGS/Vars
//-----------------------------------------------------------------------------
 
 
ANIM_HANDLE * herowaitbook = NULL;
 
ANIM_HANDLE * herowait2 = NULL;
ANIM_HANDLE * herowait_2h = NULL;
 
 

ARX_NECKLACE necklace;

long Keyring_Number = 0;
KEYRING_SLOT * Keyring = NULL;
float PLAYER_BASE_RADIUS = 52;
float PLAYER_BASE_HEIGHT = -170;
float PLAYER_CROUCH_HEIGHT = -120;
float PLAYER_LEVITATE_HEIGHT = -195;
INTERACTIVE_OBJ * CURRENT_TORCH = NULL;

unsigned long FALLING_TIME = 0;


STRUCT_QUEST * PlayerQuest = NULL;
long nb_PlayerQuest = 0;
long FistParticles = 0;
void Manage_sp_max();
bool ARX_PLAYER_IsInFightMode()
{
	if (playerCharacter.Interface & INTER_COMBATMODE) return true;

	if (inter.iobj
	        &&	(inter.iobj[0])
	        &&	(inter.iobj[0]->animlayer[1].cur_anim))
	{
		ANIM_USE * ause1 = &inter.iobj[0]->animlayer[1];
		ANIM_HANDLE ** alist = inter.iobj[0]->anims;

		if ((ause1->cur_anim	==	alist[ANIM_BARE_READY])
		        ||	(ause1->cur_anim	==	alist[ANIM_BARE_UNREADY])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_UNREADY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_UNREADY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_UNREADY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_UNREADY_PART_2])
		   )
			return true;
	}

	return false;
}
//*************************************************************************************
//*************************************************************************************
// KEYRING FUNCTIONS
//-------------------------------------------------------------------------------------

//*************************************************************************************
// void ARX_KEYRING_Init()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Init/Reset player Keyring structures
//*************************************************************************************
void ARX_KEYRING_Init()
{
	if (Keyring)
		free((void *)Keyring);

	Keyring = NULL;
	Keyring_Number = 0;
}
//*************************************************************************************
// void ARX_KEYRING_Add(char * key)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Add a key to Keyring
//*************************************************************************************
void ARX_KEYRING_Add(char * key)
{
	Keyring = (KEYRING_SLOT *)realloc(Keyring, sizeof(KEYRING_SLOT) * (Keyring_Number + 1));
	memset(&Keyring[Keyring_Number], 0, sizeof(KEYRING_SLOT));
	strcpy(Keyring[Keyring_Number].slot, key);
	Keyring_Number++;
}

//*************************************************************************************
// void ARX_KEYRING_Combine(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Sends COMBINE event to "io" for each keyring entry
//*************************************************************************************
void ARX_KEYRING_Combine(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < Keyring_Number; i++)
	{
		if (SendIOScriptEvent(io, SM_COMBINE, Keyring[i].slot) == REFUSE)
			return;
	}
}
//-----------------------------------------------------------------------------
// KEYRING FUNCTIONS end
//******************************************************************************

//-----------------------------------------------------------------------------
//*************************************************************************************
// void ARX_PLAYER_FrontPos(EERIE_3D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Fills "pos" with player "front pos" for sound purpose
//*************************************************************************************
void ARX_PLAYER_FrontPos(EERIE_3D * pos)
{
	pos->x = playerCharacter.pos.x - EEsin(DEG2RAD(MAKEANGLE(playerCharacter.angle.b))) * 100.f;
	pos->y = playerCharacter.pos.y + 100.f; //-100.f;
	pos->z = playerCharacter.pos.z + EEcos(DEG2RAD(MAKEANGLE(playerCharacter.angle.b))) * 100.f;
}

//*************************************************************************************
// void ARX_PLAYER_RectifyPosition()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Reset all extra-rotation groups of player
//*************************************************************************************
void ARX_PLAYER_RectifyPosition()
{
	INTERACTIVE_OBJ * io = inter.iobj[0];

	if ((io) && (io->_npcdata->ex_rotate))
	{
		for (long n = 0; n < MAX_EXTRA_ROTATE; n++)
		{
			io->_npcdata->ex_rotate->group_rotate[n].a = 0;
			io->_npcdata->ex_rotate->group_rotate[n].b = 0;
			io->_npcdata->ex_rotate->group_rotate[n].g = 0;
		}

		io->_npcdata->ex_rotate->flags = 0;
	}
}
//******************************************************************************
// PLAYER TORCH FUNCTIONS
//-----------------------------------------------------------------------------
void ARX_PLAYER_KillTorch()
{
	CURRENT_TORCH->show = SHOW_FLAG_IN_SCENE;
	ARX_SOUND_PlaySFX(SND_TORCH_END);
	ARX_SOUND_Stop(SND_TORCH_LOOP);

	if (CanBePutInInventory(CURRENT_TORCH))
	{
		if (CURRENT_TORCH)
			CURRENT_TORCH->show = SHOW_FLAG_IN_INVENTORY;
	}
	else
	{
		PutInFrontOfPlayer(CURRENT_TORCH, 1);	
	}

	CURRENT_TORCH = NULL;
	SHOW_TORCH = 0;
	DynLight[0].exist = 0;
}

//-----------------------------------------------------------------------------
void ARX_PLAYER_ClickedOnTorch(INTERACTIVE_OBJ * io)
{
	if (io == NULL)
	{
		return;
	}

	if (CURRENT_TORCH == NULL)
	{
		if (io->durability > 0)
		{
			if (io->ignition > 0)
			{
				if (ValidDynLight(io->ignit_light))
					DynLight[io->ignit_light].exist = 0;

				io->ignit_light = -1;

				if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
				{
					ARX_SOUND_Stop(io->ignit_sound);
					io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
				}

				io->ignition = 0;
			}

			SHOW_TORCH = 1;
			ARX_SOUND_PlaySFX(SND_TORCH_START);
			ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
			RemoveFromAllInventories(io);
			CURRENT_TORCH = io;
			io->show = SHOW_FLAG_ON_PLAYER;

			if (DRAGINTER == io)	
			{
				DRAGINTER = NULL;
			}
		}
	}
	else if (CURRENT_TORCH == io)
	{
		ARX_PLAYER_KillTorch();

	}
	else
	{
		ARX_PLAYER_KillTorch();

		if (io->durability > 0)
		{
			if (io->ignition > 0)
			{
				if (io->ignit_light != -1)
				{
					DynLight[io->ignit_light].exist = 0;
					io->ignit_light = -1;
				}

				if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
				{
					ARX_SOUND_Stop(io->ignit_sound);
					io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
				}

				io->ignition = 0;
			}

			SHOW_TORCH = 1;
			ARX_SOUND_PlaySFX(SND_TORCH_START);
			ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
			RemoveFromAllInventories(io);
			CURRENT_TORCH = io;
			io->show = SHOW_FLAG_ON_PLAYER;

			if (DRAGINTER == io)
			{
				DRAGINTER = NULL;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_PLAYER_ManageTorch()
{
	if (CURRENT_TORCH)
	{
		CURRENT_TORCH->ignition = 0;
		CURRENT_TORCH->durability -= FrameDiff * DIV10000;

		if (CURRENT_TORCH->durability <= 0)
		{

			ARX_SPEECH_ReleaseIOSpeech(CURRENT_TORCH);
			// Need To Kill timers
			ARX_SCRIPT_Timer_Clear_By_IO(CURRENT_TORCH);
			CURRENT_TORCH->show = SHOW_FLAG_KILLED;
			CURRENT_TORCH->GameFlags &= ~GFLAG_ISINTREATZONE;
			RemoveFromAllInventories(CURRENT_TORCH);
			ARX_INTERACTIVE_DestroyDynamicInfo(CURRENT_TORCH);
			ARX_SOUND_PlaySFX(SND_TORCH_END);
			ARX_SOUND_Stop(SND_TORCH_LOOP);
			ARX_INTERACTIVE_DestroyIO(CURRENT_TORCH);
			CURRENT_TORCH = NULL;
			SHOW_TORCH = 0;
			DynLight[0].exist = 0;
		}
	}
}
//-----------------------------------------------------------------------------
// PLAYER TORCH FUNCTIONS end
//******************************************************************************


//*************************************************************************************
// void ARX_PLAYER_Quest_Init()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Init/Reset player Quest structures
//*************************************************************************************
void ARX_PLAYER_Quest_Init()
{
	if (PlayerQuest)
	{
		for (long i = 0; i < nb_PlayerQuest; i++)
		{
			if (PlayerQuest[i].ident)
				free((void *)PlayerQuest[i].ident);

			if (PlayerQuest[i].localised)
				free((void *)PlayerQuest[i].localised);
		}

		free((void *)PlayerQuest);
		PlayerQuest = NULL;
	}

	nb_PlayerQuest = 0;
}

//*************************************************************************************
// void ARX_Player_Rune_Add(unsigned long _ulRune)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Add "_ulRune" to player runes
//*************************************************************************************
void ARX_Player_Rune_Add(unsigned long _ulRune)
{
	int iNbSpells = 0;
	int iNbSpellsAfter = 0;

	for (long i = 0; i < SPELL_COUNT; i++)
	{
		if (spellicons[i].bSecret == false)
		{
			long j = 0;
			bool bOk = true;

			while ((j < 4) && (spellicons[i].symbols[j] != 255))
			{
				if (!(playerCharacter.rune_flags & (1 << spellicons[i].symbols[j])))
				{
					bOk = false;
				}

				j++;
			}

			if (bOk)
			{
				iNbSpells ++;
			}
		}
	}

	playerCharacter.rune_flags |= _ulRune;

	for (int i = 0; i < SPELL_COUNT; i++)
	{
		if (spellicons[i].bSecret == false)
		{
			long j = 0;
			bool bOk = true;

			while ((j < 4) && (spellicons[i].symbols[j] != 255))
			{
				if (!(playerCharacter.rune_flags & (1 << spellicons[i].symbols[j])))
				{
					bOk = false;
				}

				j++;
			}

			if (bOk)
			{
				iNbSpellsAfter ++;
			}
		}
	}

	if (iNbSpellsAfter > iNbSpells)
	{
		MakeBookFX(DANAESIZX - INTERFACE_RATIO(35), DANAESIZY - INTERFACE_RATIO(148), 0.00001f);
		bBookHalo = true;
		ulBookHaloTime = 0;
	}
}

//*************************************************************************************
// void ARX_Player_Rune_Remove(unsigned long _ulRune
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Remove "_ulRune" from player runes
//*************************************************************************************
void ARX_Player_Rune_Remove(unsigned long _ulRune)
{
	playerCharacter.rune_flags &= ~_ulRune;
}

//*************************************************************************************
// void ARX_PLAYER_Quest_Add(char * quest)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Add quest "quest" to player Questbook
//*************************************************************************************
void ARX_PLAYER_Quest_Add(char * quest, bool _bLoad)
{
	_TCHAR output[4096];
	MakeLocalised(quest, output, 4096);

	if (output[0] == 0) return;

	PlayerQuest = (STRUCT_QUEST *)realloc(PlayerQuest, sizeof(STRUCT_QUEST) * (nb_PlayerQuest + 1));
	PlayerQuest[nb_PlayerQuest].ident = (char *)malloc(strlen(quest) + 1);
	PlayerQuest[nb_PlayerQuest].localised = (_TCHAR *)malloc((_tcslen(output) + 1) * sizeof(_TCHAR));
	strcpy(PlayerQuest[nb_PlayerQuest].ident, quest);
	_tcscpy(PlayerQuest[nb_PlayerQuest].localised, output);
	nb_PlayerQuest++;
	bBookHalo = !_bLoad;//true;
	ulBookHaloTime = 0;
}

//*************************************************************************************
// void ARX_PLAYER_Remove_Invisibility()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Removes player invisibility by killing Invisibility spells on him
//*************************************************************************************
void ARX_PLAYER_Remove_Invisibility()
{
	for (long i = 0; i < MAX_SPELLS; i++)
	{
		if ((spells[i].exist)
		        &&	(spells[i].type == SPELL_INVISIBILITY)
		        && (spells[i].caster == 0))
			spells[i].tolive = 0;
	}
}

//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Stealth(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Stealth Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Stealth(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return (float)playerCharacter.Skill_Stealth
			       + (playerCharacter.Attribute_Dexterity * 2);

		return (float)playerCharacter.Skill_Stealth + playerCharacter.Mod_Skill_Stealth
		       + (playerCharacter.Full_Attribute_Dexterity * 2);
	}

	if (type == 0)
		return (float)playerCharacter.Skill_Stealth
		       + (playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Mind);

	return (float)playerCharacter.Skill_Stealth + playerCharacter.Mod_Skill_Stealth
	       + (playerCharacter.Full_Attribute_Dexterity + playerCharacter.Full_Attribute_Mind);
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Mecanism(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Mecanism Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Mecanism(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return (float)playerCharacter.Skill_Mecanism
			       + (playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Mind);

		return (float)playerCharacter.Skill_Mecanism + playerCharacter.Mod_Skill_Mecanism
		       + (playerCharacter.Full_Attribute_Dexterity + playerCharacter.Full_Attribute_Mind);
	}

	if (type == 0)
		return (float)playerCharacter.Skill_Mecanism
		       + (playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Mind);

	return (float)playerCharacter.Skill_Mecanism + playerCharacter.Mod_Skill_Mecanism
	       + (playerCharacter.Full_Attribute_Dexterity + playerCharacter.Full_Attribute_Mind);
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Intuition(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Intuition Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Intuition(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return (float)playerCharacter.Skill_Intuition
			       + (playerCharacter.Attribute_Mind * 2);

		return (float)playerCharacter.Skill_Intuition + playerCharacter.Mod_Skill_Intuition
		       + (playerCharacter.Full_Attribute_Mind * 2);
	}

	if (type == 0)
		return (float)playerCharacter.Skill_Intuition
		       + (playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Mind);

	return (float)playerCharacter.Skill_Intuition + playerCharacter.Mod_Skill_Intuition
	       + (playerCharacter.Full_Attribute_Dexterity + playerCharacter.Full_Attribute_Mind);
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Etheral_Link(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Etheral Link Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Etheral_Link(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return (float)playerCharacter.Skill_Etheral_Link
			       + playerCharacter.Attribute_Mind * 2.f;

		return (float)playerCharacter.Skill_Etheral_Link + playerCharacter.Mod_Skill_Etheral_Link
		       + playerCharacter.Full_Attribute_Mind * 2.f;
	}

	if (type == 0)
		return (float)playerCharacter.Skill_Etheral_Link
		       + playerCharacter.Attribute_Mind * 2.f;

	return (float)playerCharacter.Skill_Etheral_Link + playerCharacter.Mod_Skill_Etheral_Link
	       + playerCharacter.Full_Attribute_Mind * 2.f;
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Object_Knowledge(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Object Knowledge Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Object_Knowledge(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return (float)playerCharacter.Skill_Object_Knowledge
			       + ((playerCharacter.Attribute_Mind * 3.f + playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Strength) * DIV2);

		return (float)playerCharacter.Skill_Object_Knowledge + playerCharacter.Mod_Skill_Object_Knowledge
		       + ((playerCharacter.Attribute_Mind * 3.f + playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Strength) * DIV2);
	}

	if (type == 0)
		return (float)playerCharacter.Skill_Object_Knowledge
		       + playerCharacter.Attribute_Mind * 2.f;

	return (float)playerCharacter.Skill_Object_Knowledge + playerCharacter.Mod_Skill_Object_Knowledge
	       + playerCharacter.Full_Attribute_Mind * 2.f;
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Casting(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Casting Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Casting(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return (float)playerCharacter.Skill_Casting
			       + playerCharacter.Attribute_Mind * 2.f ;

		return (float)playerCharacter.Skill_Casting + playerCharacter.Mod_Skill_Casting
		       + playerCharacter.Full_Attribute_Mind * 2.f ;
	}

	if (type == 0)
		return (float)playerCharacter.Skill_Casting
		       + playerCharacter.Attribute_Mind * 2.f ;

	return (float)playerCharacter.Skill_Casting + playerCharacter.Mod_Skill_Casting
	       + playerCharacter.Full_Attribute_Mind * 2.f ;
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Projectile(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Projectile Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Projectile(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return (float)playerCharacter.Skill_Projectile
			       + playerCharacter.Attribute_Dexterity * 2.f + playerCharacter.Attribute_Strength;

		return (float)playerCharacter.Skill_Projectile + playerCharacter.Mod_Skill_Projectile
		       + playerCharacter.Attribute_Dexterity * 2.f + playerCharacter.Attribute_Strength;
	}

	if (type == 0)
		return (float)playerCharacter.Skill_Projectile
		       + playerCharacter.Attribute_Dexterity * 2.f;

	return (float)playerCharacter.Skill_Projectile + playerCharacter.Mod_Skill_Projectile
	       + playerCharacter.Full_Attribute_Dexterity * 2.f;
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Close_Combat(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Close Combat Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Close_Combat(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return
			    (float)playerCharacter.Skill_Close_Combat
			    + (playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Strength * 2);

		return
		    (float)playerCharacter.Skill_Close_Combat + playerCharacter.Mod_Skill_Close_Combat
		    + (playerCharacter.Full_Attribute_Dexterity + playerCharacter.Full_Attribute_Strength * 2);
	}

	if (type == 0)
		return
		    (float)playerCharacter.Skill_Close_Combat
		    + (playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Strength);

	return
	    (float)playerCharacter.Skill_Close_Combat + playerCharacter.Mod_Skill_Close_Combat
	    + (playerCharacter.Full_Attribute_Dexterity + playerCharacter.Full_Attribute_Strength);
}
//*************************************************************************************
// float ARX_PLAYER_Get_Skill_Defense(long type)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns player Defense Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
float ARX_PLAYER_Get_Skill_Defense(long type)
{
	if (USE_NEW_SKILLS)
	{
		if (type == 0)
			return
			    (float)playerCharacter.Skill_Defense
			    + (playerCharacter.Attribute_Constitution * 3);

		return
		    (float)playerCharacter.Skill_Defense + playerCharacter.Mod_Skill_Defense
		    + (playerCharacter.Full_Attribute_Constitution * 3);
	}

	if (type == 0)
		return
		    (float)playerCharacter.Skill_Defense
		    + (playerCharacter.Attribute_Dexterity + playerCharacter.Attribute_Constitution);

	return
	    (float)playerCharacter.Skill_Defense + playerCharacter.Mod_Skill_Defense
	    + (playerCharacter.Full_Attribute_Dexterity + playerCharacter.Full_Attribute_Constitution);
}

//*************************************************************************************
// void ARX_PLAYER_ComputePlayerStats()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Compute secondary attributes for player
//*************************************************************************************
void ARX_PLAYER_ComputePlayerStats()
{
	playerCharacter.maxlife = (float)playerCharacter.Attribute_Constitution * (float)(playerCharacter.level + 2);
	playerCharacter.maxmana = (float)playerCharacter.Attribute_Mind * (float)(playerCharacter.level + 1);
	float t = ARX_PLAYER_Get_Skill_Defense(0);


	float fCalc = t * DIV10 - 1 ;
	ARX_CHECK_UCHAR(fCalc);
	playerCharacter.armor_class = ARX_CLEAN_WARN_CAST_UCHAR(fCalc);


	if (playerCharacter.armor_class < 1) playerCharacter.armor_class = 1;

	playerCharacter.damages = 100;
	playerCharacter.resist_magic = (unsigned char)(float)(playerCharacter.Attribute_Mind * 2.f
	                      * (1.f + (ARX_PLAYER_Get_Skill_Casting(0)) * DIV200));

	fCalc = playerCharacter.Attribute_Constitution * 2 + ((ARX_PLAYER_Get_Skill_Defense(1) * DIV4));
	ARX_CHECK_UCHAR(fCalc);
	playerCharacter.resist_poison = ARX_CLEAN_WARN_CAST_UCHAR(fCalc);


	playerCharacter.damages = (playerCharacter.Attribute_Strength - 10) * DIV2;

	if (playerCharacter.damages < 1.f) playerCharacter.damages = 1.f;

	playerCharacter.AimTime = 1500;
}
extern float ARX_EQUIPMENT_ApplyPercent(INTERACTIVE_OBJ * io, long ident, float trueval);
extern long cur_mr;
extern long SPECIAL_PNUX;
//*************************************************************************************
// void ARX_PLAYER_ComputePlayerFullStats()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Compute FULL versions of player stats including Equiped Items
//   and spells, and any other effect altering them.
//*************************************************************************************
void ARX_PLAYER_ComputePlayerFullStats()
{
	ARX_PLAYER_ComputePlayerStats();
	playerCharacter.Mod_Attribute_Strength = 0;
	playerCharacter.Mod_Attribute_Dexterity = 0;
	playerCharacter.Mod_Attribute_Constitution = 0;
	playerCharacter.Mod_Attribute_Mind = 0;
	playerCharacter.Mod_Skill_Stealth = 0;
	playerCharacter.Mod_Skill_Mecanism = 0;
	playerCharacter.Mod_Skill_Intuition = 0;
	playerCharacter.Mod_Skill_Etheral_Link = 0;
	playerCharacter.Mod_Skill_Object_Knowledge = 0;
	playerCharacter.Mod_Skill_Casting = 0;
	playerCharacter.Mod_Skill_Projectile = 0;
	playerCharacter.Mod_Skill_Close_Combat = 0;
	playerCharacter.Mod_Skill_Defense = 0;
	playerCharacter.Mod_armor_class = 0;
	playerCharacter.Mod_resist_magic = 0;
	playerCharacter.Mod_resist_poison = 0;
	playerCharacter.Mod_Critical_Hit = 0;
	playerCharacter.Mod_damages = 0;
	playerCharacter.Mod_life = 0;
	playerCharacter.Mod_maxlife = 0;
	playerCharacter.Mod_mana = 0;
	playerCharacter.Mod_maxmana = 0;

	ARX_EQUIPMENT_IdentifyAll();

	playerCharacter.Full_Weapon_Type = ARX_EQUIPMENT_GetPlayerWeaponType();

	INTERACTIVE_OBJ * io = inter.iobj[0];
	// Check for Equipment Modificators to Attributes
	playerCharacter.Mod_Attribute_Strength = ARX_EQUIPMENT_Apply(
	                                    io, IO_EQUIPITEM_ELEMENT_STRENGTH, playerCharacter.Attribute_Strength);
	playerCharacter.Mod_Attribute_Dexterity = ARX_EQUIPMENT_Apply(
	                                     io, IO_EQUIPITEM_ELEMENT_DEXTERITY, playerCharacter.Attribute_Dexterity);
	playerCharacter.Mod_Attribute_Constitution = ARX_EQUIPMENT_Apply(
	                                        io, IO_EQUIPITEM_ELEMENT_CONSTITUTION, playerCharacter.Attribute_Constitution);
	playerCharacter.Mod_Attribute_Mind = ARX_EQUIPMENT_Apply(
	                                io, IO_EQUIPITEM_ELEMENT_MIND, playerCharacter.Attribute_Mind);
	playerCharacter.Mod_armor_class = ARX_EQUIPMENT_Apply(
	                             io, IO_EQUIPITEM_ELEMENT_Armor_Class, playerCharacter.armor_class);

	// Check for Equipment Modificators to Skills
	playerCharacter.Mod_Skill_Stealth = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Stealth, ARX_PLAYER_Get_Skill_Stealth(0));
	playerCharacter.Mod_Skill_Mecanism = ARX_EQUIPMENT_Apply(
	                                io, IO_EQUIPITEM_ELEMENT_Mecanism, ARX_PLAYER_Get_Skill_Mecanism(0));
	playerCharacter.Mod_Skill_Intuition = ARX_EQUIPMENT_Apply(
	                                 io, IO_EQUIPITEM_ELEMENT_Intuition, ARX_PLAYER_Get_Skill_Intuition(0));
	playerCharacter.Mod_Skill_Etheral_Link = ARX_EQUIPMENT_Apply(
	                                    io, IO_EQUIPITEM_ELEMENT_Etheral_Link, ARX_PLAYER_Get_Skill_Etheral_Link(0));
	playerCharacter.Mod_Skill_Object_Knowledge = ARX_EQUIPMENT_Apply(
	                                        io, IO_EQUIPITEM_ELEMENT_Object_Knowledge, ARX_PLAYER_Get_Skill_Object_Knowledge(0));
	playerCharacter.Mod_Skill_Casting = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Casting, ARX_PLAYER_Get_Skill_Casting(0));
	playerCharacter.Mod_Skill_Projectile = ARX_EQUIPMENT_Apply(
	                                  io, IO_EQUIPITEM_ELEMENT_Projectile, ARX_PLAYER_Get_Skill_Projectile(0));
	playerCharacter.Mod_Skill_Close_Combat = ARX_EQUIPMENT_Apply(
	                                    io, IO_EQUIPITEM_ELEMENT_Close_Combat, ARX_PLAYER_Get_Skill_Close_Combat(0));
	playerCharacter.Mod_Skill_Defense = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Defense, ARX_PLAYER_Get_Skill_Defense(0));

	playerCharacter.Mod_resist_magic = ARX_EQUIPMENT_Apply(
	                              io, IO_EQUIPITEM_ELEMENT_Resist_Magic, playerCharacter.resist_magic);
	playerCharacter.Mod_resist_poison = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Resist_Poison, playerCharacter.resist_poison);
	playerCharacter.Mod_Critical_Hit = ARX_EQUIPMENT_Apply(
	                              io, IO_EQUIPITEM_ELEMENT_Critical_Hit, playerCharacter.Critical_Hit);
	playerCharacter.Mod_damages = ARX_EQUIPMENT_Apply(
	                         io, IO_EQUIPITEM_ELEMENT_Damages, 0); 


	//CHECK OVERFLOW
	float fFullAimTime	= ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_AimTime, 0);
	float fCalcHandicap	= (playerCharacter.Full_Attribute_Dexterity - 10.f) * 20.f;
	ARX_CHECK_LONG(fFullAimTime);
	ARX_CHECK_LONG(fCalcHandicap);

	//CAST
	playerCharacter.Full_AimTime = ARX_CLEAN_WARN_CAST_LONG(fFullAimTime);

	if (playerCharacter.Full_AimTime <= 0) playerCharacter.Full_AimTime = playerCharacter.AimTime;

	playerCharacter.Full_AimTime -= ARX_CLEAN_WARN_CAST_LONG(fCalcHandicap);


	if (playerCharacter.Full_AimTime <= 1500) playerCharacter.Full_AimTime = 1500;



	/// PERCENTILE.....
	playerCharacter.Mod_Attribute_Strength += ARX_EQUIPMENT_ApplyPercent(
	                                     io, IO_EQUIPITEM_ELEMENT_STRENGTH, playerCharacter.Attribute_Strength + playerCharacter.Mod_Attribute_Strength);
	playerCharacter.Mod_Attribute_Dexterity += ARX_EQUIPMENT_ApplyPercent(
	                                      io, IO_EQUIPITEM_ELEMENT_DEXTERITY, playerCharacter.Attribute_Dexterity + playerCharacter.Mod_Attribute_Dexterity);
	playerCharacter.Mod_Attribute_Constitution += ARX_EQUIPMENT_ApplyPercent(
	        io, IO_EQUIPITEM_ELEMENT_CONSTITUTION, playerCharacter.Attribute_Constitution + playerCharacter.Mod_Attribute_Constitution);
	playerCharacter.Mod_Attribute_Mind += ARX_EQUIPMENT_ApplyPercent(
	                                 io, IO_EQUIPITEM_ELEMENT_MIND, playerCharacter.Attribute_Mind + playerCharacter.Mod_Attribute_Mind);
	playerCharacter.Mod_armor_class += ARX_EQUIPMENT_ApplyPercent(
	                              io, IO_EQUIPITEM_ELEMENT_Armor_Class, playerCharacter.armor_class + playerCharacter.Mod_armor_class);

	// Check for Equipment Modificators to Skills
	playerCharacter.Mod_Skill_Stealth += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Stealth, ARX_PLAYER_Get_Skill_Stealth(1));
	playerCharacter.Mod_Skill_Mecanism += ARX_EQUIPMENT_ApplyPercent(
	                                 io, IO_EQUIPITEM_ELEMENT_Mecanism, ARX_PLAYER_Get_Skill_Mecanism(1));
	playerCharacter.Mod_Skill_Intuition += ARX_EQUIPMENT_ApplyPercent(
	                                  io, IO_EQUIPITEM_ELEMENT_Intuition, ARX_PLAYER_Get_Skill_Intuition(1));
	playerCharacter.Mod_Skill_Etheral_Link += ARX_EQUIPMENT_ApplyPercent(
	                                     io, IO_EQUIPITEM_ELEMENT_Etheral_Link, ARX_PLAYER_Get_Skill_Etheral_Link(1));
	playerCharacter.Mod_Skill_Object_Knowledge += ARX_EQUIPMENT_ApplyPercent(
	        io, IO_EQUIPITEM_ELEMENT_Object_Knowledge, ARX_PLAYER_Get_Skill_Object_Knowledge(1));
	playerCharacter.Mod_Skill_Casting += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Casting, ARX_PLAYER_Get_Skill_Casting(1));
	playerCharacter.Mod_Skill_Projectile += ARX_EQUIPMENT_ApplyPercent(
	                                   io, IO_EQUIPITEM_ELEMENT_Projectile, ARX_PLAYER_Get_Skill_Projectile(1));
	playerCharacter.Mod_Skill_Close_Combat += ARX_EQUIPMENT_ApplyPercent(
	                                     io, IO_EQUIPITEM_ELEMENT_Close_Combat, ARX_PLAYER_Get_Skill_Close_Combat(1));
	playerCharacter.Mod_Skill_Defense += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Defense, ARX_PLAYER_Get_Skill_Defense(1));

	playerCharacter.Mod_resist_magic += ARX_EQUIPMENT_ApplyPercent(
	                               io, IO_EQUIPITEM_ELEMENT_Resist_Magic, playerCharacter.resist_magic + playerCharacter.Mod_resist_magic);
	playerCharacter.Mod_resist_poison += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Resist_Poison, playerCharacter.resist_poison + playerCharacter.Mod_resist_poison);
	playerCharacter.Mod_Critical_Hit += ARX_EQUIPMENT_ApplyPercent(
	                               io, IO_EQUIPITEM_ELEMENT_Critical_Hit, playerCharacter.Critical_Hit + playerCharacter.Mod_Critical_Hit);
	playerCharacter.Mod_damages += ARX_EQUIPMENT_ApplyPercent(
	                          io, IO_EQUIPITEM_ELEMENT_Damages, playerCharacter.damages);
	//player.Full_AimTime=ARX_EQUIPMENT_ApplyPercent(
	//	io,IO_EQUIPITEM_ELEMENT_AimTime,0);



	// Check for Spell Modificators
	if (inter.iobj[0])
		for (long i = 0; i < inter.iobj[0]->nb_spells_on; i++)
		{
			long n = inter.iobj[0]->spells_on[i];

			if (spells[n].exist)
			{
				switch (spells[n].type)
				{
					case SPELL_ARMOR:
						playerCharacter.Mod_armor_class += spells[n].caster_level;
						break;
					case SPELL_LOWER_ARMOR:
						playerCharacter.Mod_armor_class -= spells[n].caster_level;
						break;
					case SPELL_CURSE:
						playerCharacter.Mod_Attribute_Strength -= spells[n].caster_level;
						playerCharacter.Mod_Attribute_Constitution -= spells[n].caster_level;
						playerCharacter.Mod_Attribute_Dexterity -= spells[n].caster_level;
						playerCharacter.Mod_Attribute_Mind -= spells[n].caster_level;
						break;
					case SPELL_BLESS:
						playerCharacter.Mod_Attribute_Strength += spells[n].caster_level;
						playerCharacter.Mod_Attribute_Dexterity += spells[n].caster_level;
						playerCharacter.Mod_Attribute_Constitution += spells[n].caster_level;
						playerCharacter.Mod_Attribute_Mind += spells[n].caster_level;
						break;
				}
			}
		}

	if (cur_mr == 3)
	{
		playerCharacter.Mod_Attribute_Strength += 1;
		playerCharacter.Mod_Attribute_Mind += 10;
		playerCharacter.Mod_Attribute_Constitution += 1;
		playerCharacter.Mod_Attribute_Dexterity += 10;
		playerCharacter.Mod_Skill_Stealth += 5;
		playerCharacter.Mod_Skill_Mecanism += 5;
		playerCharacter.Mod_Skill_Intuition += 100;
		playerCharacter.Mod_Skill_Etheral_Link += 100;
		playerCharacter.Mod_Skill_Object_Knowledge += 100;
		playerCharacter.Mod_Skill_Casting += 5;
		playerCharacter.Mod_Skill_Projectile += 5;
		playerCharacter.Mod_Skill_Close_Combat += 5;
		playerCharacter.Mod_Skill_Defense += 100;
		playerCharacter.Mod_resist_magic += 100;
		playerCharacter.Mod_resist_poison += 100;
		playerCharacter.Mod_Critical_Hit += 5;
		playerCharacter.Mod_damages += 2;
		playerCharacter.Mod_armor_class += 100;
		playerCharacter.Full_AimTime = 100;
	}

	if (sp_max)
	{
		playerCharacter.Mod_Attribute_Strength += 5;
		playerCharacter.Mod_Attribute_Mind += 5;
		playerCharacter.Mod_Attribute_Constitution += 5;
		playerCharacter.Mod_Attribute_Dexterity += 5;
		playerCharacter.Mod_Skill_Stealth += 50;
		playerCharacter.Mod_Skill_Mecanism += 50;
		playerCharacter.Mod_Skill_Intuition += 50;
		playerCharacter.Mod_Skill_Etheral_Link += 50;
		playerCharacter.Mod_Skill_Object_Knowledge += 50;
		playerCharacter.Mod_Skill_Casting += 50;
		playerCharacter.Mod_Skill_Projectile += 50;
		playerCharacter.Mod_Skill_Close_Combat += 50;
		playerCharacter.Mod_Skill_Defense += 50;
		playerCharacter.Mod_resist_magic += 10;
		playerCharacter.Mod_resist_poison += 10;
		playerCharacter.Mod_Critical_Hit += 50;
		playerCharacter.Mod_damages += 10;
		playerCharacter.Mod_armor_class += 20;
		playerCharacter.Full_AimTime = 100;
	}

	if (SPECIAL_PNUX)
	{
		playerCharacter.Mod_Attribute_Strength += (long)(float)(rnd() * 5.f);
		playerCharacter.Mod_Attribute_Mind += (long)(float)(rnd() * 5.f);
		playerCharacter.Mod_Attribute_Constitution += (long)(float)(rnd() * 5.f);
		playerCharacter.Mod_Attribute_Dexterity += (long)(float)(rnd() * 5.f);
		playerCharacter.Mod_Skill_Stealth += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Mecanism += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Intuition += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Etheral_Link += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Object_Knowledge += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Casting += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Projectile += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Close_Combat += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Skill_Defense += (long)(float)(rnd() * 30.f);
		playerCharacter.Mod_resist_magic += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_resist_poison += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_Critical_Hit += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_damages += (long)(float)(rnd() * 20.f);
		playerCharacter.Mod_armor_class += (long)(float)(rnd() * 20.f);
	}

	if (cur_rf == 3)
	{
		playerCharacter.Mod_Attribute_Mind += 10;
		playerCharacter.Mod_Skill_Casting += 100;
		playerCharacter.Mod_Skill_Etheral_Link += 100;
		playerCharacter.Mod_Skill_Object_Knowledge += 100;
		playerCharacter.Mod_resist_magic += 20;
		playerCharacter.Mod_resist_poison += 20;
		playerCharacter.Mod_damages += 1;
		playerCharacter.Mod_armor_class += 5;
	}

	playerCharacter.Full_armor_class = playerCharacter.armor_class + playerCharacter.Mod_armor_class;

	if (playerCharacter.Full_armor_class < 0) playerCharacter.Full_armor_class = 0;

	playerCharacter.Full_Attribute_Strength = playerCharacter.Attribute_Strength + playerCharacter.Mod_Attribute_Strength;

	if (playerCharacter.Full_Attribute_Strength < 0) playerCharacter.Full_Attribute_Strength = 0;

	playerCharacter.Full_Attribute_Mind = playerCharacter.Attribute_Mind + playerCharacter.Mod_Attribute_Mind;

	if (playerCharacter.Full_Attribute_Mind < 0) playerCharacter.Full_Attribute_Mind = 0;

	playerCharacter.Full_Attribute_Constitution = playerCharacter.Attribute_Constitution + playerCharacter.Mod_Attribute_Constitution;

	if (playerCharacter.Full_Attribute_Constitution < 0) playerCharacter.Full_Attribute_Constitution = 0;

	playerCharacter.Full_Attribute_Dexterity = playerCharacter.Attribute_Dexterity + playerCharacter.Mod_Attribute_Dexterity;

	if (playerCharacter.Full_Attribute_Dexterity < 0) playerCharacter.Full_Attribute_Dexterity = 0;

	playerCharacter.Full_Skill_Stealth = ARX_PLAYER_Get_Skill_Stealth(1); 
	playerCharacter.Full_Skill_Mecanism = ARX_PLAYER_Get_Skill_Mecanism(1);
	playerCharacter.Full_Skill_Intuition = ARX_PLAYER_Get_Skill_Intuition(1); 
	playerCharacter.Full_Skill_Etheral_Link = ARX_PLAYER_Get_Skill_Etheral_Link(1); 
	playerCharacter.Full_Skill_Object_Knowledge = ARX_PLAYER_Get_Skill_Object_Knowledge(1); 
	playerCharacter.Full_Skill_Casting = ARX_PLAYER_Get_Skill_Casting(1); 
	playerCharacter.Full_Skill_Projectile = ARX_PLAYER_Get_Skill_Projectile(1); 
	playerCharacter.Full_Skill_Close_Combat = ARX_PLAYER_Get_Skill_Close_Combat(1); 
	playerCharacter.Full_Skill_Defense = ARX_PLAYER_Get_Skill_Defense(1); 

	playerCharacter.Full_resist_magic = playerCharacter.resist_magic + playerCharacter.Mod_resist_magic;

	if (playerCharacter.Full_resist_magic < 0) playerCharacter.Full_resist_magic = 0;

	playerCharacter.Full_resist_poison = playerCharacter.resist_poison + playerCharacter.Mod_resist_poison;

	if (playerCharacter.Full_resist_poison < 0) playerCharacter.Full_resist_poison = 0;

	playerCharacter.Full_Critical_Hit = playerCharacter.Critical_Hit + playerCharacter.Mod_Critical_Hit;

	if (playerCharacter.Full_Critical_Hit < 0) playerCharacter.Full_Critical_Hit = 0;

	playerCharacter.Full_damages = playerCharacter.damages + playerCharacter.Mod_damages
	                      + playerCharacter.Full_Skill_Close_Combat * DIV10;

	if (playerCharacter.Full_damages < 1) playerCharacter.Full_damages = 1;

	playerCharacter.Full_life = playerCharacter.life;
	playerCharacter.Full_mana = playerCharacter.mana;
	playerCharacter.Full_maxlife = (float)playerCharacter.Full_Attribute_Constitution * (float)(playerCharacter.level + 2) + playerCharacter.Mod_maxlife;
	playerCharacter.life = __min(playerCharacter.life, playerCharacter.Full_maxlife);
	playerCharacter.Full_maxmana = (float)playerCharacter.Full_Attribute_Mind * (float)(playerCharacter.level + 1) + playerCharacter.Mod_maxmana;
	playerCharacter.mana = __min(playerCharacter.mana, playerCharacter.Full_maxmana);
}

//*************************************************************************************
// void ARX_PLAYER_MakeFreshHero()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Creates a "Fresh" hero
//*************************************************************************************
void ARX_PLAYER_MakeFreshHero()
{
	playerCharacter.Attribute_Strength = 6;
	playerCharacter.Attribute_Mind = 6;
	playerCharacter.Attribute_Dexterity = 6;
	playerCharacter.Attribute_Constitution = 6;

	playerCharacter.Old_Skill_Stealth			=	playerCharacter.Skill_Stealth			= 0;
	playerCharacter.Old_Skill_Mecanism			=	playerCharacter.Skill_Mecanism			= 0;
	playerCharacter.Old_Skill_Intuition			=	playerCharacter.Skill_Intuition			= 0;
	playerCharacter.Old_Skill_Etheral_Link		=	playerCharacter.Skill_Etheral_Link		= 0;
	playerCharacter.Old_Skill_Object_Knowledge	=	playerCharacter.Skill_Object_Knowledge	= 0;
	playerCharacter.Old_Skill_Casting			=	playerCharacter.Skill_Casting			= 0;
	playerCharacter.Old_Skill_Projectile			=	playerCharacter.Skill_Projectile			= 0;
	playerCharacter.Old_Skill_Close_Combat		=	playerCharacter.Skill_Close_Combat		= 0;
	playerCharacter.Old_Skill_Defense			=	playerCharacter.Skill_Defense			= 0;

	playerCharacter.Attribute_Redistribute = 16;
	playerCharacter.Skill_Redistribute = 18; 

	playerCharacter.level = 0;
	playerCharacter.xp = 0;
	playerCharacter.poison = 0.f;
	playerCharacter.hunger = 100.f;
	playerCharacter.skin = 0;
	playerCharacter.bag = 1;

	ARX_PLAYER_ComputePlayerStats();
	playerCharacter.rune_flags = 0;

	playerCharacter.SpellToMemorize.bSpell = false;
}
char SKIN_MOD = 0;
char QUICK_MOD = 0;

void ARX_SPSound()
{
	ARX_SOUND_PlayCinematic("kra_zoha_equip.wav");
}
//*************************************************************************************
void ARX_PLAYER_MakeSpHero()
{
	ARX_SPSound();
	playerCharacter.Attribute_Strength = 12;
	playerCharacter.Attribute_Mind = 12;
	playerCharacter.Attribute_Dexterity = 12;
	playerCharacter.Attribute_Constitution = 12;

	playerCharacter.Old_Skill_Stealth			=	playerCharacter.Skill_Stealth			= 5;
	playerCharacter.Old_Skill_Mecanism			=	playerCharacter.Skill_Mecanism			= 5;
	playerCharacter.Old_Skill_Intuition			=	playerCharacter.Skill_Intuition			= 5;
	playerCharacter.Old_Skill_Etheral_Link		=	playerCharacter.Skill_Etheral_Link		= 5;
	playerCharacter.Old_Skill_Object_Knowledge	=	playerCharacter.Skill_Object_Knowledge	= 5;
	playerCharacter.Old_Skill_Casting			=	playerCharacter.Skill_Casting			= 5;
	playerCharacter.Old_Skill_Projectile			=	playerCharacter.Skill_Projectile			= 5;
	playerCharacter.Old_Skill_Close_Combat		=	playerCharacter.Skill_Close_Combat		= 5;
	playerCharacter.Old_Skill_Defense			=	playerCharacter.Skill_Defense			= 5;

	playerCharacter.Attribute_Redistribute = 6;
	playerCharacter.Skill_Redistribute = 10;

	playerCharacter.level = 1;
	playerCharacter.xp = 0;
	playerCharacter.poison = 0.f;
	playerCharacter.hunger = 100.f;
	playerCharacter.skin = 4;

	ARX_PLAYER_ComputePlayerStats();
	playerCharacter.life = playerCharacter.maxlife;
	playerCharacter.mana = playerCharacter.maxmana;

	playerCharacter.rune_flags = 0xFFFFFFFF;
	playerCharacter.SpellToMemorize.bSpell = false;

	SKIN_MOD = 0;
	QUICK_MOD = 0;
}
//*************************************************************************************
// void ARX_PLAYER_MakePowerfullHero()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Creates a "POWERFULL" hero
//*************************************************************************************
void ARX_PLAYER_MakePowerfullHero()
{
	playerCharacter.Attribute_Strength = 18;
	playerCharacter.Attribute_Mind = 18;
	playerCharacter.Attribute_Dexterity = 18;
	playerCharacter.Attribute_Constitution = 18;

	playerCharacter.Old_Skill_Stealth			=	playerCharacter.Skill_Stealth			= 82;
	playerCharacter.Old_Skill_Mecanism			=	playerCharacter.Skill_Mecanism			= 82;
	playerCharacter.Old_Skill_Intuition			=	playerCharacter.Skill_Intuition			= 82;
	playerCharacter.Old_Skill_Etheral_Link		=	playerCharacter.Skill_Etheral_Link		= 82;
	playerCharacter.Old_Skill_Object_Knowledge	=	playerCharacter.Skill_Object_Knowledge	= 82;
	playerCharacter.Old_Skill_Casting			=	playerCharacter.Skill_Casting			= 82;
	playerCharacter.Old_Skill_Projectile			=	playerCharacter.Skill_Projectile			= 82;
	playerCharacter.Old_Skill_Close_Combat		=	playerCharacter.Skill_Close_Combat		= 82;
	playerCharacter.Old_Skill_Defense			=	playerCharacter.Skill_Defense			= 82;

	playerCharacter.Attribute_Redistribute = 0;
	playerCharacter.Skill_Redistribute = 0;

	playerCharacter.level = 10;
	playerCharacter.xp = 178000;
	playerCharacter.poison = 0.f;
	playerCharacter.hunger = 100.f;
	playerCharacter.skin = 0;

	ARX_PLAYER_ComputePlayerStats();
	playerCharacter.life = playerCharacter.maxlife;
	playerCharacter.mana = playerCharacter.maxmana;

	playerCharacter.rune_flags = 0xFFFFFFFF;
	playerCharacter.SpellToMemorize.bSpell = false;
}

//*************************************************************************************
// void ARX_PLAYER_MakeAverageHero()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Creates an "Average" hero
//*************************************************************************************
void ARX_PLAYER_MakeAverageHero()
{
	ARX_PLAYER_MakeFreshHero();

	playerCharacter.Attribute_Strength		+= 4;
	playerCharacter.Attribute_Mind			+= 4;
	playerCharacter.Attribute_Dexterity		+= 4;
	playerCharacter.Attribute_Constitution	+= 4;

	playerCharacter.Skill_Stealth			+= 2;
	playerCharacter.Skill_Mecanism			+= 2;
	playerCharacter.Skill_Intuition			+= 2;
	playerCharacter.Skill_Etheral_Link		+= 2;
	playerCharacter.Skill_Object_Knowledge	+= 2;
	playerCharacter.Skill_Casting			+= 2;
	playerCharacter.Skill_Projectile			+= 2;
	playerCharacter.Skill_Close_Combat		+= 2;
	playerCharacter.Skill_Defense			+= 2;

	playerCharacter.Attribute_Redistribute = 0;
	playerCharacter.Skill_Redistribute = 0;

	playerCharacter.level = 0;
	playerCharacter.xp = 0;
	playerCharacter.hunger = 100.f;

	ARX_PLAYER_ComputePlayerStats();
}

//*************************************************************************************
// void ARX_PLAYER_QuickGeneration()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Quickgenerate a random hero
//*************************************************************************************
void ARX_PLAYER_QuickGeneration()
{
	long old_skin = playerCharacter.skin;
	ARX_PLAYER_MakeFreshHero();


	ARX_CHECK_CHAR(old_skin);
	playerCharacter.skin	= ARX_CLEAN_WARN_CAST_CHAR(old_skin);

	while (playerCharacter.Attribute_Redistribute)
	{
		float rn = rnd();

		if ((rn < 0.25f) && (playerCharacter.Attribute_Strength < 18))
		{
			playerCharacter.Attribute_Strength++;
			playerCharacter.Attribute_Redistribute--;
		}
		else if ((rn < 0.5f) && (playerCharacter.Attribute_Mind < 18))
		{
			playerCharacter.Attribute_Mind++;
			playerCharacter.Attribute_Redistribute--;
		}
		else if ((rn < 0.75f) && (playerCharacter.Attribute_Dexterity < 18))
		{
			playerCharacter.Attribute_Dexterity++;
			playerCharacter.Attribute_Redistribute--;
		}
		else if (playerCharacter.Attribute_Constitution < 18)
		{
			playerCharacter.Attribute_Constitution++;
			playerCharacter.Attribute_Redistribute--;
		}
	}

	while (playerCharacter.Skill_Redistribute)
	{
		float rn = rnd();

		if ((rn < 0.1f) && (playerCharacter.Skill_Stealth < 18))
		{
			playerCharacter.Skill_Stealth++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.2f) && (playerCharacter.Skill_Mecanism < 18))
		{
			playerCharacter.Skill_Mecanism++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.3f) && (playerCharacter.Skill_Intuition < 18))
		{
			playerCharacter.Skill_Intuition++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.4f) && (playerCharacter.Skill_Etheral_Link < 18))
		{
			playerCharacter.Skill_Etheral_Link++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.5f) && (playerCharacter.Skill_Object_Knowledge < 18))
		{
			playerCharacter.Skill_Object_Knowledge++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.6f) && (playerCharacter.Skill_Casting < 18))
		{
			playerCharacter.Skill_Casting++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.7f) && (playerCharacter.Skill_Projectile < 18))
		{
			playerCharacter.Skill_Projectile++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.8f) && (playerCharacter.Skill_Close_Combat < 18))
		{
			playerCharacter.Skill_Close_Combat++;
			playerCharacter.Skill_Redistribute--;
		}
		else if ((rn < 0.9f) && (playerCharacter.Skill_Defense < 18))
		{
			playerCharacter.Skill_Defense++;
			playerCharacter.Skill_Redistribute--;
		}
	}

	playerCharacter.level = 0;
	playerCharacter.xp = 0;
	playerCharacter.hunger = 100.f;

	ARX_PLAYER_ComputePlayerStats();
}

//*************************************************************************************
// long GetXPforLevel(long level)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns necessary Experience for a given level
//*************************************************************************************
long GetXPforLevel(long level)
{
	switch (level)
	{
		case 0:
			return 0;
			break;
		case 1:
			return 2000;
			break;
		case 2:
			return 4000;
			break;
		case 3:
			return 6000;
			break;
		case 4:
			return 10000;
			break;
		case 5:
			return 16000;
			break;
		case 6:
			return 26000;
			break;
		case 7:
			return 42000;
			break;
		case 8:
			return 68000;
			break;
		case 9:
			return 110000;
			break;
		case 10:
			return 178000;
			break;
		case 11:
			return 300000;
			break;
		case 12:
			return 450000;
			break;
		case 13:
			return 600000;
			break;
		case 14:
			return 750000;
			break;
		default:
			return level * 60000;
	}

	return LONG_MAX;
}

//*************************************************************************************
// void ARX_PLAYER_LEVEL_UP()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Manages Player Level Up event
//*************************************************************************************
void ARX_PLAYER_LEVEL_UP()
{
	ARX_SOUND_PlayInterface(SND_PLAYER_LEVEL_UP);
	playerCharacter.level++;
	playerCharacter.Skill_Redistribute += 15; 
	playerCharacter.Attribute_Redistribute++;
	ARX_PLAYER_ComputePlayerStats();
	playerCharacter.life = playerCharacter.maxlife;
	playerCharacter.mana = playerCharacter.maxmana;
	playerCharacter.Old_Skill_Stealth			=	playerCharacter.Skill_Stealth;
	playerCharacter.Old_Skill_Mecanism			=	playerCharacter.Skill_Mecanism;
	playerCharacter.Old_Skill_Intuition			=	playerCharacter.Skill_Intuition;
	playerCharacter.Old_Skill_Etheral_Link		=	playerCharacter.Skill_Etheral_Link;
	playerCharacter.Old_Skill_Object_Knowledge	=	playerCharacter.Skill_Object_Knowledge;
	playerCharacter.Old_Skill_Casting			=	playerCharacter.Skill_Casting;
	playerCharacter.Old_Skill_Projectile			=	playerCharacter.Skill_Projectile;
	playerCharacter.Old_Skill_Close_Combat		=	playerCharacter.Skill_Close_Combat;
	playerCharacter.Old_Skill_Defense			=	playerCharacter.Skill_Defense;
	SendIOScriptEvent(inter.iobj[0], 0, "", "LEVEL_UP");
}

//*************************************************************************************
// void ARX_PLAYER_Modify_XP(long val)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Modify player XP by adding "val" to it
//*************************************************************************************
void ARX_PLAYER_Modify_XP(long val)
{

	playerCharacter.xp += val;				

	for (long i = 1; i < 11; i++)
	{
		long lvup = 0;

		if (i > playerCharacter.level)
		{
			if ((playerCharacter.xp >= GetXPforLevel(i)))
			{
				lvup = 1;
			}

			if (lvup) ARX_PLAYER_LEVEL_UP();
		}
	}
}

//*************************************************************************************
// void ARX_PLAYER_Poison(float val)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Function to poison player by "val" poison level
//*************************************************************************************
void ARX_PLAYER_Poison(float val)
{
	// Make a poison saving throw to see if player is affected
	if (rnd() * 100.f > playerCharacter.resist_poison)
	{
		playerCharacter.poison += val;
		ARX_SOUND_PlayInterface(SND_PLAYER_POISONED);
	}
}
long PLAYER_PARALYSED = 0;
//*************************************************************************************
// void ARX_PLAYER_FrameCheck(float _framedelay)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   updates some player stats depending on time:
//		.life/mana recovery
//		.poison evolution
//		.hunger check
//		.invisibility
//*************************************************************************************
void ARX_PLAYER_FrameCheck(float Framedelay)
{
	//	ARX_PLAYER_QuickGeneration();
	if (Framedelay > 0)
	{
		UpdateIOInvisibility(inter.iobj[0]);
		// Natural LIFE recovery
		float inc = 0.00008f * Framedelay * (playerCharacter.Full_Attribute_Constitution + playerCharacter.Full_Attribute_Strength * DIV2 + playerCharacter.Full_Skill_Defense) * DIV50;

		if (playerCharacter.life > 0.f)
		{
			float inc_hunger = 0.00008f * Framedelay * (playerCharacter.Full_Attribute_Constitution + playerCharacter.Full_Attribute_Strength * DIV2) * DIV50;

			// Check for player hungry sample playing
			if (((playerCharacter.hunger > 10.f) && (playerCharacter.hunger - inc_hunger <= 10.f))
			        || ((playerCharacter.hunger < 10.f) && (ARXTime > LastHungerSample + 180000)))
			{
				LastHungerSample = ARXTimeUL();

				if (!BLOCK_PLAYER_CONTROLS)
				{
					bool bOk = true;

					for (long i = 0; i < MAX_ASPEECH; i++)
					{
						if (aspeech[i].exist && (aspeech[i].io == inter.iobj[0]))
						{
							bOk = false;
						}
					}

					if (bOk)
						ARX_SPEECH_AddSpeech(inter.iobj[0], "[Player_Off_Hungry]", PARAM_LOCALISED, ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
				}
			}

			playerCharacter.hunger -= inc_hunger * .5f; //*.7f;

			if (playerCharacter.hunger < -10.f) playerCharacter.hunger = -10.f;

			if (!BLOCK_PLAYER_CONTROLS)
			{
				if (playerCharacter.hunger < 0.f) playerCharacter.life -= inc * DIV2;
				else playerCharacter.life += inc;
			}

			// Natural MANA recovery
			playerCharacter.mana += 0.00008f * Framedelay * ((playerCharacter.Full_Attribute_Mind + playerCharacter.Full_Skill_Etheral_Link) * 10) * DIV100; //_framedelay*DIV1000;

			if (playerCharacter.mana > playerCharacter.Full_maxmana) playerCharacter.mana = playerCharacter.Full_maxmana;
		}

		//float pmaxlife=(float)player.Full_Attribute_Constitution*(float)(player.level+2);
		if (playerCharacter.life > playerCharacter.Full_maxlife) playerCharacter.life = playerCharacter.Full_maxlife;

		// Now Checks Poison Progression
		if (!BLOCK_PLAYER_CONTROLS)
			if (playerCharacter.poison > 0.f)
			{
				float cp = playerCharacter.poison;
				cp *= DIV2 * Framedelay * DIV1000 * DIV2;
				float faster = 10.f - playerCharacter.poison;

				if (faster < 0.f) faster = 0.f;

				if (rnd() * 100.f > playerCharacter.resist_poison + faster)
				{
					float dmg = cp * DIV3;

					if (playerCharacter.life - dmg <= 0.f) ARX_DAMAGES_DamagePlayer(dmg, DAMAGE_TYPE_POISON, -1, NULL);
					else playerCharacter.life -= dmg;

					playerCharacter.poison -= cp * DIV10;
				}
				else playerCharacter.poison -= cp;
			}

		if (playerCharacter.poison < 0.1f) playerCharacter.poison = 0.f;
	}
}
TextureContainer * PLAYER_SKIN_TC = NULL;

void	ARX_PLAYER_Restore_Skin()
{

	char tx[256];
	char tx2[256];
	char tx3[256];
	char tx4[256];
	tx[0] = 0;
	tx2[0] = 0;
	tx3[0] = 0;
	tx4[0] = 0;

	switch (playerCharacter.skin)
	{
		case 0:
			strcpy(tx, "Graph\\Obj3D\\Textures\\npc_human_base_hero_head.bmp");
			strcpy(tx2, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero_head.bmp");
			strcpy(tx3, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero_head.bmp");
			strcpy(tx4, "Graph\\Obj3D\\Textures\\npc_human_leather_hero_head.bmp");
			break;
		case 1:
			strcpy(tx, "Graph\\Obj3D\\Textures\\npc_human_base_hero2_head.bmp");
			strcpy(tx2, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero2_head.bmp");
			strcpy(tx3, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero2_head.bmp");
			strcpy(tx4, "Graph\\Obj3D\\Textures\\npc_human_leather_hero2_head.bmp");
			break;
		case 2:
			strcpy(tx, "Graph\\Obj3D\\Textures\\npc_human_base_hero3_head.bmp");
			strcpy(tx2, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero3_head.bmp");
			strcpy(tx3, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero3_head.bmp");
			strcpy(tx4, "Graph\\Obj3D\\Textures\\npc_human_leather_hero3_head.bmp");
			break;
		case 3:
			strcpy(tx, "Graph\\Obj3D\\Textures\\npc_human_base_hero4_head.bmp");
			strcpy(tx2, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero4_head.bmp");
			strcpy(tx3, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero4_head.bmp");
			strcpy(tx4, "Graph\\Obj3D\\Textures\\npc_human_leather_hero4_head.bmp");
			break;
		case 4:
			strcpy(tx, "Graph\\Obj3D\\Textures\\npc_human_cm_hero_head.bmp");
			strcpy(tx2, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero_head.bmp");
			strcpy(tx3, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero_head.bmp");
			strcpy(tx4, "Graph\\Obj3D\\Textures\\npc_human_leather_hero_head.bmp");
			break;
		case 5:
			strcpy(tx, "Graph\\Obj3D\\Textures\\npc_human__base_hero_head.bmp");
			strcpy(tx2, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero_head.bmp");
			strcpy(tx3, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero_head.bmp");
			strcpy(tx4, "Graph\\Obj3D\\Textures\\npc_human_leather_hero_head.bmp");
			break;
		case 6: //just in case
			strcpy(tx, "Graph\\Obj3D\\Textures\\npc_human__base_hero_head.bmp");
			strcpy(tx2, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero_head.bmp");
			strcpy(tx3, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero_head.bmp");
			strcpy(tx4, "Graph\\Obj3D\\Textures\\npc_human_leather_hero_head.bmp");
			break;
	}

	if (PLAYER_SKIN_TC)
	{

		if (tx[0])
		{
			MakeUpcase(tx);
			sprintf((char *)PLAYER_SKIN_TC->m_strName, "%s%s", Project.workingdir, (char *)tx);
			ReloadTexture(PLAYER_SKIN_TC);
			PLAYER_SKIN_TC->Restore(GDevice);
		}
	}

	char tt[256];
	strcpy(tt, "Graph\\Obj3D\\Textures\\npc_human_chainmail_hero_head.bmp");
	MakeUpcase(tt);
	TextureContainer * tmpTC = _FindTexture(tt);

	if ((tmpTC) && (tx2[0]))
	{
		MakeUpcase(tx2);
		sprintf((char *)tmpTC->m_strName, "%s%s", Project.workingdir, (char *)tx2);
		ReloadTexture(tmpTC);
		tmpTC->Restore(GDevice);
	}

	strcpy(tt, "Graph\\Obj3D\\Textures\\npc_human_chainmail_mithril_hero_head.bmp");
	MakeUpcase(tt);
	tmpTC = _FindTexture(tt);

	if ((tmpTC) && (tx3[0]))
	{
		MakeUpcase(tx3);
		sprintf((char *)tmpTC->m_strName, "%s%s", Project.workingdir, (char *)tx3);
		ReloadTexture(tmpTC);
		tmpTC->Restore(GDevice);
	}

	strcpy(tt, "Graph\\Obj3D\\Textures\\npc_human_leather_hero_head.bmp");
	MakeUpcase(tt);
	tmpTC = _FindTexture(tt);

	if ((tmpTC) && (tx4[0]))
	{
		MakeUpcase(tx4);
		sprintf((char *)tmpTC->m_strName, "%s%s", Project.workingdir, (char *)tx4);
		ReloadTexture(tmpTC);
		tmpTC->Restore(GDevice);
	}
}
extern void LaunchMoulinex();
extern HRESULT DANAEFinalCleanup();
//*************************************************************************************
// void ARX_PLAYER_LoadHeroAnimsAndMesh()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Load Mesh & anims for hero
//*************************************************************************************
void ARX_PLAYER_LoadHeroAnimsAndMesh()
{
	char tex[256];
	char tex1[256];
	sprintf(tex1, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);
	MakeDir(tex, "graph\\Obj3D\\Interactive\\NPC\\human_base\\human_base.teo");
 
	hero = TheoToEerie_Fast(tex1, tex, TTE_NO_PHYSICS_BOX | TTE_NPC);
	PLAYER_SKIN_TC = MakeTCFromFile("Graph\\Obj3D\\Textures\\npc_human_base_hero_head.bmp");

	MakeDir(tex, "graph\\Obj3D\\Anims\\NPC\\human_wait_book.tea");
	herowaitbook = EERIE_ANIMMANAGER_Load(tex);
	MakeDir(tex, "graph\\Obj3D\\Anims\\NPC\\human_normal_wait.tea");
	herowait2 = EERIE_ANIMMANAGER_Load(tex);
	MakeDir(tex, "graph\\Obj3D\\Anims\\NPC\\human_wait_book_2handed.tea");
	herowait_2h = EERIE_ANIMMANAGER_Load(tex);

	INTERACTIVE_OBJ * io = CreateFreeInter(0);
	io->obj = hero; 

	playerCharacter.skin = 0;
	ARX_PLAYER_Restore_Skin();

	ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0], 0);
	ARX_INTERACTIVE_HideGore(inter.iobj[0], 1);
	io->ident = -1;

	//todo free
	io->_npcdata = (IO_NPCDATA *)malloc(sizeof(IO_NPCDATA));
	memset(io->_npcdata, 0, sizeof(IO_NPCDATA));
	io->_npcdata->blood_color = 0xFFFF0000;
	io->ioflags = IO_NPC;
	io->_npcdata->maxlife = io->_npcdata->life = 10;
	io->_npcdata->vvpos = -99999.f;
	io->_npcdata->speakpitch = 1.f;

	//todo free
	io->armormaterial = strdup("LEATHER");
	sprintf(io->filename, "%sgraph\\obj3D\\Interactive\\Player\\Player.teo", Project.workingdir);
	char texscript[256];
	strcpy(texscript, io->filename);
	SetExt(texscript, ".asl");

	if (PAK_FileExist(texscript))
	{
		long FileSize = 0;
		io->script.data = (char *)PAK_FileLoadMalloc(texscript, &FileSize);

		if (io->script.data != NULL)
		{
			io->script.size = FileSize;
			InitScript(&io->script);
		}
	}
	else
	{
		io->script.size = 0;
		io->script.data = NULL;
	}


	if ((EERIE_OBJECT_GetGroup(io->obj, "head") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "neck") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "chest") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "belt") != -1))
	{
		io->_npcdata->ex_rotate = (EERIE_EXTRA_ROTATE *)malloc(sizeof(EERIE_EXTRA_ROTATE));

		if (io->_npcdata->ex_rotate)
		{
			io->_npcdata->ex_rotate->group_number[0] = (short)EERIE_OBJECT_GetGroup(io->obj, "head");
			io->_npcdata->ex_rotate->group_number[1] = (short)EERIE_OBJECT_GetGroup(io->obj, "neck");
			io->_npcdata->ex_rotate->group_number[2] = (short)EERIE_OBJECT_GetGroup(io->obj, "chest");
			io->_npcdata->ex_rotate->group_number[3] = (short)EERIE_OBJECT_GetGroup(io->obj, "belt");

			for (long n = 0; n < MAX_EXTRA_ROTATE; n++)
			{
				io->_npcdata->ex_rotate->group_rotate[n].a = 0;
				io->_npcdata->ex_rotate->group_rotate[n].b = 0;
				io->_npcdata->ex_rotate->group_rotate[n].g = 0;
			}

			io->_npcdata->ex_rotate->flags = 0;
		}
	}

	ARX_INTERACTIVE_RemoveGoreOnIO(inter.iobj[0]); 
}
float Falling_Height = 0;
void ARX_PLAYER_StartFall()
{
	FALLING_TIME = ARXTimeUL(); 
	Falling_Height = 50.f;
	float yy;
	EERIEPOLY * ep = CheckInPoly(playerCharacter.pos.x, playerCharacter.pos.y, playerCharacter.pos.z, &yy);

	if (ep)
	{
		Falling_Height = playerCharacter.pos.y; 
	}
}

//*************************************************************************************
// void ARX_PLAYER_BecomesDead()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Called When player has just died
//*************************************************************************************
void ARX_PLAYER_BecomesDead()
{
	STARTED_A_GAME = 0;
	// a mettre au final
	BLOCK_PLAYER_CONTROLS = 1;

	if (inter.iobj[0])
	{
		playerCharacter.Interface &= ~INTER_COMBATMODE;
		playerCharacter.Interface = 0;
		DeadCameraDistance = 40.f;
		DeadTime = 0;
	}

	for (long i = 0; i < MAX_SPELLS; i++)
	{
		if ((spells[i].exist)
		        &&	(spells[i].caster == 0))
		{
			spells[i].tolive = 0;
		}
	}
}
float LASTPLAYERA = 0;
extern long ON_PLATFORM;
long LAST_ON_PLATFORM = 0;
extern long MOVE_PRECEDENCE;
extern long EXTERNALVIEW;
long NOT_MOVED_AT_ALL = 0;
//*************************************************************************************
// void ARX_PLAYER_Manage_Visual()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Manages Player visual
//   Choose the set of animations to use to represent current player
//   situation.
//*************************************************************************************
void ARX_PLAYER_Manage_Visual()
{

	unsigned long tim = ARXTimeUL();

	if (playerCharacter.Current_Movement & PLAYER_ROTATE)
	{
		if (ROTATE_START == 0)
		{
			ROTATE_START = tim;
		}
	}
	else
	{
		if (ROTATE_START)
		{
			float diff = (float)tim - (float)ROTATE_START;

			if (diff > 100)
				ROTATE_START = 0;
		}
	}

	static long special[3];
	long light = 0;

	if (inter.iobj[0])
	{
		INTERACTIVE_OBJ * io = inter.iobj[0];

		if (!BLOCK_PLAYER_CONTROLS)
			if (sp_max)
			{
				io->halo.color.r = 1.f;
				io->halo.color.g = 0.f;
				io->halo.color.b = 0.f;
				io->halo.flags |= HALO_ACTIVE | HALO_DYNLIGHT;
				io->halo.radius = 20.f;
				playerCharacter.life += (float)FrameDiff * DIV10; 
				playerCharacter.life = __min(playerCharacter.life, playerCharacter.maxlife);
				playerCharacter.mana += (float)FrameDiff * DIV10; 
				playerCharacter.mana = __min(playerCharacter.mana, playerCharacter.maxmana);

			}

		if (cur_mr == 3)
		{
			playerCharacter.life += (float)FrameDiff * DIV20; 
			playerCharacter.life = __min(playerCharacter.life, playerCharacter.maxlife);
			playerCharacter.mana += (float)FrameDiff * DIV20; 
			playerCharacter.mana = __min(playerCharacter.mana, playerCharacter.maxmana);
		}

		io->pos.x = playerCharacter.pos.x;
		io->pos.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
		io->pos.z = playerCharacter.pos.z;

		if ((playerCharacter.jumpphase == 0) && (!LAST_ON_PLATFORM))
		{
			float tempo;

			EERIEPOLY * ep = CheckInPolyPrecis(playerCharacter.pos.x, playerCharacter.pos.y, playerCharacter.pos.z, &tempo);

			if (ep && ((io->pos.y > tempo - 30.f) && (io->pos.y < tempo)))
			{
					playerCharacter.onfirmground = 1;
			}
		}

		ComputeVVPos(io);
		io->pos.y = io->_npcdata->vvpos;

		if ((!(playerCharacter.Current_Movement & PLAYER_CROUCH)) && (playerCharacter.physics.cyl.height > -150.f))
		{
			float old = playerCharacter.physics.cyl.height;
			playerCharacter.physics.cyl.height = PLAYER_BASE_HEIGHT;
			playerCharacter.physics.cyl.origin.x = playerCharacter.pos.x;
			playerCharacter.physics.cyl.origin.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
			playerCharacter.physics.cyl.origin.z = playerCharacter.pos.z;
			float anything = CheckAnythingInCylinder(&playerCharacter.physics.cyl, inter.iobj[0]); 

			if (anything < 0.f)
			{
				playerCharacter.Current_Movement |= PLAYER_CROUCH;
				playerCharacter.physics.cyl.height = old;
			}
		}

		if (playerCharacter.life > 0)
		{
			io->angle.a = 0;
			io->angle.b = 180.f - playerCharacter.angle.b; //+90.f;
			io->angle.g = 0;
		}

		io->GameFlags |= GFLAG_ISINTREATZONE;

		ANIM_USE * ause0 = &io->animlayer[0];
		ANIM_USE * ause1 = &io->animlayer[1];
 
		ANIM_USE * ause3 = &io->animlayer[3];
		ause0->next_anim = NULL;
		inter.iobj[0]->animlayer[1].next_anim = NULL;
		inter.iobj[0]->animlayer[2].next_anim = NULL;
		inter.iobj[0]->animlayer[3].next_anim = NULL;
		ANIM_HANDLE ** alist = io->anims;

		if (ause0->flags & EA_FORCEPLAY)
		{
			if (ause0->flags & EA_ANIMEND)
			{
				ause0->flags &= ~EA_FORCEPLAY;
				ause0->flags |= EA_STATICANIM;
				io->move.x = io->lastmove.x = 0.f;
				io->move.y = io->lastmove.y = 0.f;
				io->move.z = io->lastmove.z = 0.f;
			}
			else
			{
				ause0->flags &= ~EA_STATICANIM;
				io->pos.x = playerCharacter.pos.x = moveto.x = playerCharacter.pos.x + io->move.x;
				io->pos.y = playerCharacter.pos.y = moveto.y = playerCharacter.pos.y + io->move.y;
				io->pos.z = playerCharacter.pos.z = moveto.z = playerCharacter.pos.z + io->move.z;
				io->pos.y += -PLAYER_BASE_HEIGHT;
				goto nochanges;
			}
		}

		ANIM_HANDLE * ChangeMoveAnim = NULL;
		ANIM_HANDLE * ChangeMoveAnim2 = NULL;
		long ChangeMA_Loop = 1;
		long ChangeMA_Stopend = 0;
		long NO_Interpolation = 0;

		if (io->ioflags & IO_FREEZESCRIPT) goto nochanges;

		if (playerCharacter.life <= 0)
		{
			HERO_SHOW_1ST = -1;
			io->animlayer[1].cur_anim = NULL;
			ChangeMoveAnim = alist[ANIM_DIE];
			ChangeMA_Loop = 0;
			ChangeMA_Stopend = 1; 
			goto makechanges;
		}

		if (playerCharacter.Current_Movement == 0 || playerCharacter.Current_Movement == PLAYER_MOVE_STEALTH)
		{
			if (playerCharacter.Interface & INTER_COMBATMODE)
				ChangeMoveAnim = alist[ANIM_FIGHT_WAIT];
			else if (EXTERNALVIEW)
				ChangeMoveAnim = alist[ANIM_WAIT];
			else
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];

			ChangeMA_Loop = 1;
		}

		if (playerCharacter.Current_Movement & PLAYER_ROTATE)
		{
			if (playerCharacter.Interface & INTER_COMBATMODE)
				ChangeMoveAnim = alist[ANIM_FIGHT_WAIT];
			else	if (EXTERNALVIEW)
				ChangeMoveAnim = alist[ANIM_WAIT];
			else
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];

			ChangeMA_Loop = 1;
		}

		if ((ROTATE_START) && (playerCharacter.angle.a > 60.f) && (playerCharacter.angle.a < 180.f) && (LASTPLAYERA > 60.f) && (LASTPLAYERA < 180.f))
		{
			NO_Interpolation = 1;

			if (PLAYER_ROTATION < 0)
			{
				if (playerCharacter.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_U_TURN_LEFT_FIGHT];
				else	ChangeMoveAnim = alist[ANIM_U_TURN_LEFT];
			}
			else
			{
				if (playerCharacter.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT_FIGHT];
				else	ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT];
			}

			ChangeMA_Loop = 1;

			if ((ause0->cur_anim == alist[ANIM_U_TURN_LEFT])
			        ||	(ause0->cur_anim == alist[ANIM_U_TURN_LEFT_FIGHT]))
			{
				float fv = PLAYER_ROTATION * 5;
				long vv;
				F2L(fv, &vv);
				io->frameloss -= fv - (float)vv;

				if (io->frameloss < 0) io->frameloss = 0;

				ause0->ctime -= vv;

				if (ause0->ctime < 0)
					ause0->ctime = 0;
			}
			else if ((ause0->cur_anim == alist[ANIM_U_TURN_RIGHT])
			         ||	(ause0->cur_anim == alist[ANIM_U_TURN_RIGHT_FIGHT]))
			{
				long vv;
				F2L(PLAYER_ROTATION * 5, &vv);
				float fv = PLAYER_ROTATION * 5;
				F2L(fv, &vv);
				io->frameloss += fv - (float)vv;

				if (io->frameloss < 0) io->frameloss = 0;

				ause0->ctime += vv; 

				if (ause0->ctime < 0)
					ause0->ctime = 0;
			}
		}

		LASTPLAYERA = playerCharacter.angle.a;
	
		{
			long tmove = playerCharacter.Current_Movement;

			if (((tmove & PLAYER_MOVE_STRAFE_LEFT)
			        && (tmove & PLAYER_MOVE_STRAFE_RIGHT)))
			{
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;
			}

			if (MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_LEFT)
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;

			if (MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_RIGHT)
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;

			if (MOVE_PRECEDENCE == PLAYER_MOVE_WALK_FORWARD)
				tmove &= ~PLAYER_MOVE_WALK_BACKWARD;

			if (playerCharacter.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				tmove = PLAYER_MOVE_WALK_FORWARD;

			{
				if (tmove & PLAYER_MOVE_STRAFE_LEFT)
				{
					if (playerCharacter.Interface & INTER_COMBATMODE)
						ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_LEFT];
					else if (playerCharacter.Current_Movement & PLAYER_MOVE_STEALTH)
						ChangeMoveAnim = alist[ANIM_STRAFE_LEFT];
					else
						ChangeMoveAnim = alist[ANIM_STRAFE_RUN_LEFT];
				}

				if (tmove & PLAYER_MOVE_STRAFE_RIGHT)
				{
					if (playerCharacter.Interface & INTER_COMBATMODE)
						ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_RIGHT];
					else if (playerCharacter.Current_Movement & PLAYER_MOVE_STEALTH)
						ChangeMoveAnim = alist[ANIM_STRAFE_RIGHT];
					else
						ChangeMoveAnim = alist[ANIM_STRAFE_RUN_RIGHT];
				}
			}

			if (tmove & PLAYER_MOVE_WALK_BACKWARD)
			{
				if (playerCharacter.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_BACKWARD];
				else if (playerCharacter.Current_Movement & PLAYER_MOVE_STEALTH)
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				else if (playerCharacter.Current_Movement & PLAYER_CROUCH)
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				else
					ChangeMoveAnim = alist[ANIM_RUN_BACKWARD];
			}

			if (tmove & PLAYER_MOVE_WALK_FORWARD)
			{

				if (playerCharacter.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_FORWARD];
				else if (playerCharacter.Current_Movement & PLAYER_MOVE_STEALTH)
					ChangeMoveAnim = alist[ANIM_WALK];
				else
					ChangeMoveAnim = alist[ANIM_RUN];
			}

		}

		if (ChangeMoveAnim == NULL)
		{
			if (EXTERNALVIEW)
				ChangeMoveAnim = alist[ANIM_WAIT];
			else
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];

			ChangeMA_Loop = 1;
		}

		// Finally update anim
		if ((ause1->cur_anim == NULL)
		        &&	((ause0->cur_anim == alist[ANIM_WAIT])
		             ||	(ause0->cur_anim == alist[ANIM_WAIT_SHORT]))
		        &&	(!(playerCharacter.Current_Movement & PLAYER_CROUCH)))
		{
			if ((playerCharacter.Current_Movement & PLAYER_LEAN_LEFT)
			        &&	(playerCharacter.Current_Movement & PLAYER_LEAN_RIGHT))
			{
			}
			else
			{
				if (playerCharacter.Current_Movement & PLAYER_LEAN_LEFT)
				{
					ChangeMoveAnim2 = alist[ANIM_LEAN_LEFT];
					//ChangeMA_Loop=0;
				}

				if (playerCharacter.Current_Movement & PLAYER_LEAN_RIGHT)
				{
					ChangeMoveAnim2 = alist[ANIM_LEAN_RIGHT];
				}
			}
		}

		if ((ChangeMoveAnim2 == NULL)
		        &&	(ause3->cur_anim)
		        &&	((ause3->cur_anim == alist[ANIM_LEAN_RIGHT])
		             ||	(ause3->cur_anim == alist[ANIM_LEAN_LEFT]))
		   )
		{
			AcquireLastAnim(io);
			ause3->cur_anim = NULL;
		}

		if ((playerCharacter.Current_Movement & PLAYER_CROUCH) && !(playerCharacter.Last_Movement & PLAYER_CROUCH)
		        && !playerCharacter.levitate)
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_START];
			ChangeMA_Loop = 0;
		}
		else if (!(playerCharacter.Current_Movement & PLAYER_CROUCH) && (playerCharacter.Last_Movement & PLAYER_CROUCH))
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_END];
			ChangeMA_Loop = 0;
		}
		else if (playerCharacter.Current_Movement & PLAYER_CROUCH)
		{
			if (ause0->cur_anim == alist[ANIM_CROUCH_START])
			{
				if (!(ause0->flags & EA_ANIMEND))
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_START];
					ChangeMA_Loop = 0;
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = 1;
					playerCharacter.physics.cyl.height = PLAYER_CROUCH_HEIGHT;
				}
			}
			else
			{
				if ((ChangeMoveAnim == alist[ANIM_STRAFE_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_STRAFE_RUN_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_LEFT])
				   )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_LEFT];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_STRAFE_RIGHT])
				         ||	(ChangeMoveAnim == alist[ANIM_STRAFE_RUN_RIGHT])
				         ||	(ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_RIGHT])
				        )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_RIGHT];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_WALK])
				         ||	(ChangeMoveAnim == alist[ANIM_RUN])
				         ||	(ChangeMoveAnim == alist[ANIM_FIGHT_WALK_FORWARD])
				        )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_WALK_BACKWARD])
				         || (ChangeMoveAnim == alist[ANIM_FIGHT_WALK_BACKWARD])
				        )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK_BACKWARD];
					ChangeMA_Loop = 1;
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = 1;
				}
			}
		}

		if (ause0->cur_anim == alist[ANIM_CROUCH_END])
		{
			if (!(ause0->flags & EA_ANIMEND))
				goto nochanges;
		}

	retry:
		;

		if (ARX_SPELLS_ExistAnyInstance(SPELL_FLYING_EYE))
		{
			FistParticles |= 1;
		}
		else FistParticles &= ~1;

		if (FistParticles)
		{
			light = 1;

			if (FistParticles & 1)
			{
				ChangeMoveAnim = alist[ANIM_MEDITATION];
				ChangeMA_Loop = 1;
			}

			EERIE_3DOBJ * eobj = io->obj;
			long pouet = 2;

			while (pouet)
			{
				long id;

				if (pouet == 2)
					id = io->obj->fastaccess.primary_attach;
				else
					id = GetActionPointIdx(io->obj, "LEFT_ATTACH"); 

				pouet--;

				if (id != -1)
				{
					if (special[pouet] == -1)
					{
						special[pouet] = GetFreeDynLight();
					}

					if (special[pouet] != -1)
					{
						EERIE_LIGHT * el = &DynLight[special[pouet]];
						el->intensity = 1.3f;
						el->exist = 1;
						el->fallend = 180.f;
						el->fallstart = 50.f;

						if (FistParticles & 2)
						{
							el->rgb.r = 1.f;
							el->rgb.g = 0.3f;
							el->rgb.b = 0.2f;
						}
						else
						{
							el->rgb.r = 0.7f;
							el->rgb.g = 0.3f;
							el->rgb.b = 1.f;
						}

						el->pos.x = eobj->vertexlist3[id].v.x;
						el->pos.y = eobj->vertexlist3[id].v.y;
						el->pos.z = eobj->vertexlist3[id].v.z;
					}
					else
					{
						EERIE_LIGHT * el = &DynLight[special[pouet]];
						el->intensity = 1.3f + rnd() * 0.2f;
						el->fallend = 175.f + rnd() * 10.f;

						if (FistParticles & 2)
						{
							el->fallstart *= 2.f;
							el->fallend *= 3.f;
						}
					}

					for (long kk = 0; kk < 2; kk++)
					{
						EERIE_3D  target;
						target.x = eobj->vertexlist3[id].v.x;
						target.y = eobj->vertexlist3[id].v.y;
						target.z = eobj->vertexlist3[id].v.z;
						long j = ARX_PARTICLES_GetFree();

						if ((j != -1) && (!ARXPausedTimer))
						{
							ParticleCount++;
							particle[j].exist = TRUE;
							particle[j].zdec = 0;

 
							particle[j].ov.x	=	target.x + 1.f - rnd() * 2.f;
							particle[j].ov.y	=	target.y + 1.f - rnd() * 2.f;
							particle[j].ov.z	=	target.z + 1.f - rnd() * 2.f;
							particle[j].move.x	=	0.1f - 0.2f * rnd();
							particle[j].move.y	=	-2.2f * rnd();
							particle[j].move.z	=	0.1f - 0.2f * rnd();
							particle[j].siz		=	5.f;
							particle[j].tolive	=	1500 + (unsigned long)(float)(rnd() * 2000.f);
							particle[j].scale.x	=	0.2f;
							particle[j].scale.y	=	0.2f;
							particle[j].scale.z	=	0.2f;
							particle[j].timcreation	=	lARXTime;
							particle[j].tc		=	TC_smoke;
							particle[j].special	=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;// | SUBSTRACT;
							particle[j].sourceionum	=	0;
							particle[j].source	=	&eobj->vertexlist3[id].v;
							particle[j].fparam	=	0.0000001f;

							if (FistParticles & 2)
							{
								particle[j].move.y *= 2.f;
								particle[j].r = 1.f - rnd() * 0.1f;
								particle[j].g = 0.3f + rnd() * 0.1f;
								particle[j].b = 0.2f - rnd() * 0.1f;
							}
							else
							{
								particle[j].r = 0.7f - rnd() * 0.1f;
								particle[j].g = 0.3f - rnd() * 0.1f;
								particle[j].b = 1.f - rnd() * 0.1f;
							}
						}
					}
				}
			}

			goto makechanges;
		}
		else if (ARX_SPELLS_GetSpellOn(io, SPELL_LEVITATE) >= 0)
		{
			ChangeMoveAnim = alist[ANIM_LEVITATE];
			ChangeMA_Loop = 1;
			goto makechanges;
		}
		else if (playerCharacter.jumpphase)
		{
			switch (playerCharacter.jumpphase)
			{
				case 1: // Anticipation
					FALLING_TIME = 0;
					Full_Jump_Height = 0;
					playerCharacter.jumpphase = 2;
					ChangeMoveAnim = alist[ANIM_JUMP_UP];
					ChangeMA_Stopend = 1;
					playerCharacter.jumpstarttime = ARXTimeUL();
					playerCharacter.jumplastposition = -1.f;
					break;
				case 2: // Moving Up
					ChangeMoveAnim = alist[ANIM_JUMP_UP];
					ChangeMA_Stopend = 1;

					if (playerCharacter.jumplastposition >= 1.f)
					{
						playerCharacter.jumpphase = 4;
						ChangeMoveAnim = alist[ANIM_JUMP_CYCLE];
						ARX_PLAYER_StartFall();
					}
					break;
				case 4: // Post-synch
					NO_Interpolation = 1;
					ChangeMA_Stopend = 1;
					LAST_JUMP_ENDTIME = ARXTimeUL();

					if (((ause0->cur_anim == alist[ANIM_JUMP_END])
					        && (ause0->flags & EA_ANIMEND))
					        || playerCharacter.onfirmground)
					{
						playerCharacter.jumpphase = 5;
						ChangeMoveAnim = alist[ANIM_JUMP_END_PART2];
					}
					else ChangeMoveAnim = alist[ANIM_JUMP_END];

					break;
				case 5: // Post-synch
					NO_Interpolation = 1;
					LAST_JUMP_ENDTIME = ARXTimeUL();

					if ((ause0->cur_anim == alist[ANIM_JUMP_END_PART2])
					        && (ause0->flags & EA_ANIMEND))
					{
						AcquireLastAnim(io);
						playerCharacter.jumpphase = 0;

						goto retry;
					}
					else if ((ause0->cur_anim == alist[ANIM_JUMP_END_PART2])
					         && ((EEfabs(playerCharacter.physics.velocity.x) + EEfabs(playerCharacter.physics.velocity.z)) > 4)
					         && (ause0->ctime > 1))
					{
						AcquireLastAnim(io);
						playerCharacter.jumpphase = 0;
						goto retry;
					}
					else ChangeMoveAnim = alist[ANIM_JUMP_END_PART2];

					break;
			}

			if ((ChangeMoveAnim) && (ChangeMoveAnim != ause0->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if ((ChangeMoveAnim == alist[ANIM_U_TURN_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]))
					ause0->flags |= EA_EXCONTROL;
			}

			if ((ChangeMoveAnim2) && (ChangeMoveAnim2 != ause3->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		}
		else
		{
		makechanges:
			;

			if ((ChangeMoveAnim) && (ChangeMoveAnim != ause0->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if (ChangeMA_Loop)		ause0->flags |= EA_LOOP;

				if (ChangeMA_Stopend)	ause0->flags |= EA_STOPEND;

				if ((ChangeMoveAnim == alist[ANIM_U_TURN_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]))
					ause0->flags |= EA_EXCONTROL;
			}

			if ((ChangeMoveAnim2) && (ChangeMoveAnim2 != ause3->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		}

		memcpy(&io->physics, &playerCharacter.physics, sizeof(IO_PHYSICS));
	}

nochanges:
	;
	playerCharacter.Last_Movement = playerCharacter.Current_Movement;

	if (!light)
	{
		if (special[2] != -1)
		{
			DynLight[special[2]].exist = 0;
			special[2] = -1;
		}

		if (special[1] != -1)
		{
			DynLight[special[1]].exist = 0;
			special[1] = -1;
		}
	}
}
//*************************************************************************************
// void InitPlayer()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Init Local Player Data
//*************************************************************************************
void ARX_PLAYER_InitPlayer()
{
	char tex[256];
	strcpy(tex, "Beware of the EXE that sleeps... :) ");
	playerCharacter.Interface = INTER_MINIBOOK | INTER_MINIBACK | INTER_LIFE_MANA;
	playerCharacter.physics.cyl.height = PLAYER_BASE_HEIGHT;
	playerCharacter.physics.cyl.radius = PLAYER_BASE_RADIUS;
	playerCharacter.life = playerCharacter.maxlife = playerCharacter.Full_maxlife = 100.f;
	playerCharacter.mana = playerCharacter.maxmana = playerCharacter.Full_maxmana = 100.f;
	playerCharacter.falling = 0;
	playerCharacter.rightIO = NULL;
	playerCharacter.leftIO = NULL;
	playerCharacter.equipsecondaryIO = NULL;
	playerCharacter.equipshieldIO = NULL;
	playerCharacter.gold = 0;
	playerCharacter.bag = 1;
	playerCharacter.doingmagic = 0;
	ARX_PLAYER_MakeFreshHero();
}

//*************************************************************************************
// void ForcePlayerLookAtIO(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Forces player orientation to look at an IO
//*************************************************************************************
void ForcePlayerLookAtIO(INTERACTIVE_OBJ * io)
{
	// Validity Check
	if (!io) return;

	EERIE_CAMERA tcam;
	EERIE_3D target;

	long id = inter.iobj[0]->obj->fastaccess.view_attach; 

	if (id != -1)
	{
		tcam.pos.x = inter.iobj[0]->obj->vertexlist3[id].v.x;
		tcam.pos.y = inter.iobj[0]->obj->vertexlist3[id].v.y;
		tcam.pos.z = inter.iobj[0]->obj->vertexlist3[id].v.z;
	}
	else
	{
		tcam.pos.x = playerCharacter.pos.x;
		tcam.pos.y = playerCharacter.pos.y;
		tcam.pos.z = playerCharacter.pos.z;
	}

	id = io->obj->fastaccess.view_attach; 

	if (id != -1)
	{
		target.x = io->obj->vertexlist3[id].v.x;
		target.y = io->obj->vertexlist3[id].v.y;
		target.z = io->obj->vertexlist3[id].v.z;
	}
	else
	{
		target.x = io->pos.x;
		target.y = io->pos.y;
		target.z = io->pos.z;
	}

	// For the case of not already computed Vlist3... !
	if (EEDistance3D(&target, &io->pos) > 400.f)
	{
		target.x = io->pos.x;
		target.y = io->pos.y;
		target.z = io->pos.z;
	}

	SetTargetCamera(&tcam, target.x, target.y, target.z);
	playerCharacter.desiredangle.a = playerCharacter.angle.a = MAKEANGLE(-tcam.angle.a);
	playerCharacter.desiredangle.b = playerCharacter.angle.b = MAKEANGLE(tcam.angle.b - 180.f);
	playerCharacter.angle.g = 0;
}
extern float PLAYER_ARMS_FOCAL;
extern long CURRENT_BASE_FOCAL;
extern long TRAP_DETECT;
extern long TRAP_SECRET;

extern EERIE_3D TVCONTROLEDplayerpos;
extern long TVCONTROLED;
extern long FINAL_RELEASE;


//*************************************************************************************
// void ARX_PLAYER_Frame_Update()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Updates Many player infos each frame...
//*************************************************************************************
void ARX_PLAYER_Frame_Update()
{
	if (ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_PARALYSE) >= 0)
	{
		PLAYER_PARALYSED = 1;
	}
	else
	{
		inter.iobj[0]->ioflags &= ~IO_FREEZESCRIPT; 
		PLAYER_PARALYSED = 0;
	}

	// EDITMODE: Forced Changepos for Player by Treeview Command
	if (TVCONTROLED)
	{
		playerCharacter.pos.x = TVCONTROLEDplayerpos.x;
		playerCharacter.pos.y = TVCONTROLEDplayerpos.y;
		playerCharacter.pos.z = TVCONTROLEDplayerpos.z;
		TVCONTROLED = 0;
	}

	// Reset player moveto info
	moveto.x = playerCharacter.pos.x;
	moveto.y = playerCharacter.pos.y;
	moveto.z = playerCharacter.pos.z;

	// Reset current movement flags
	playerCharacter.Current_Movement = 0;

	// Updates player angles to desired angles
	playerCharacter.angle.a = playerCharacter.desiredangle.a;
	playerCharacter.angle.b = playerCharacter.desiredangle.b;

	// Updates player Extra-Rotate Informations
	INTERACTIVE_OBJ * io;
	io = inter.iobj[0];

	if ((io) && (io->_npcdata->ex_rotate))
	{
		float v = playerCharacter.angle.a;

		if (v > 160) v = -(360 - v);

		if (playerCharacter.Interface & INTER_COMBATMODE)
		{
			if (ARX_EQUIPMENT_GetPlayerWeaponType() == WEAPON_BOW)
			{
				io->_npcdata->ex_rotate->group_rotate[0].a = 0; //head
				io->_npcdata->ex_rotate->group_rotate[1].a = 0; //neck
				io->_npcdata->ex_rotate->group_rotate[2].a = 0; //chest
				io->_npcdata->ex_rotate->group_rotate[3].a = v; //belt
			}
			else 
			{
				v *= DIV10; 
				io->_npcdata->ex_rotate->group_rotate[0].a = v; //head
				io->_npcdata->ex_rotate->group_rotate[1].a = v; //neck
				io->_npcdata->ex_rotate->group_rotate[2].a = v * 4; //chest
				io->_npcdata->ex_rotate->group_rotate[3].a = v * 4; //belt
			}

		}
		else
		{
			v *= DIV4; 
			io->_npcdata->ex_rotate->group_rotate[0].a = v; //head
			io->_npcdata->ex_rotate->group_rotate[1].a = v; //neck
			io->_npcdata->ex_rotate->group_rotate[2].a = v; //chest
			io->_npcdata->ex_rotate->group_rotate[3].a = v; //belt*/
		}

		if ((playerCharacter.Interface & INTER_COMBATMODE) || (playerCharacter.doingmagic == 2))
			io->_npcdata->ex_rotate->flags &= ~EXTRA_ROTATE_REALISTIC;
	}

	// Changes player ARMS focal depending on alpha angle.
	if (playerCharacter.angle.a > 180)
		PLAYER_ARMS_FOCAL = (float)CURRENT_BASE_FOCAL - 80.f; 
	else
		PLAYER_ARMS_FOCAL = (float)CURRENT_BASE_FOCAL - 80.f + playerCharacter.angle.a;

	PLAYER_ARMS_FOCAL = ARX_CLEAN_WARN_CAST_FLOAT(CURRENT_BASE_FOCAL);

	
	ARX_PLAYER_ComputePlayerFullStats();

	ARX_CHECK_LONG(playerCharacter.Full_Skill_Mecanism);
	ARX_CHECK_LONG(playerCharacter.Full_Skill_Intuition);

	TRAP_DETECT	=	ARX_CLEAN_WARN_CAST_LONG(playerCharacter.Full_Skill_Mecanism);
	TRAP_SECRET	=	ARX_CLEAN_WARN_CAST_LONG(playerCharacter.Full_Skill_Intuition);


	if (ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_DETECT_TRAP) >= 0)
		TRAP_DETECT = 100;

	ModeLight |= MODE_DEPTHCUEING;

	ARX_PLAYER_ManageTorch();
}
//*************************************************************************************
// void ARX_PLAYER_MakeStepNoise()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Emit player step noise
//*************************************************************************************
void ARX_PLAYER_MakeStepNoise()
{
	if (ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_LEVITATE) >= 0)
		return;

	if (USE_PLAYERCOLLISIONS)
	{
		float volume = ARX_NPC_AUDIBLE_VOLUME_DEFAULT;
		float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT;

		if (playerCharacter.Current_Movement & PLAYER_MOVE_STEALTH)
		{
			float skill_stealth;

			skill_stealth = playerCharacter.Full_Skill_Stealth / ARX_PLAYER_SKILL_STEALTH_MAX;

			volume -= ARX_NPC_AUDIBLE_VOLUME_RANGE * skill_stealth;
			factor += ARX_NPC_AUDIBLE_FACTOR_RANGE * skill_stealth;
		}

		EERIE_3D pos;

		pos.x = playerCharacter.pos.x;
		pos.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
		pos.z = playerCharacter.pos.z;

		ARX_NPC_NeedStepSound(inter.iobj[0], &pos, volume, factor);
	}

	while (currentdistance >= STEP_DISTANCE) currentdistance -= STEP_DISTANCE;
}
extern bool bGCroucheToggle;
extern float MAX_ALLOWED_PER_SECOND;

long LAST_VECT_COUNT = -1;
long LAST_FIRM_GROUND = 1;
long TRUE_FIRM_GROUND = 1;
long DISABLE_JUMP = 0;
float lastposy = -9999999.f;
unsigned long REQUEST_JUMP = 0;
extern float GLOBAL_SLOWDOWN;
extern float Original_framedelay;

unsigned long LAST_JUMP_ENDTIME = 0;
bool Valid_Jump_Pos()
{
	if ((LAST_ON_PLATFORM) || playerCharacter.climbing)
		return true;

	EERIE_CYLINDER tmpp;
	tmpp.height = playerCharacter.physics.cyl.height;
	tmpp.origin.x = playerCharacter.pos.x;
	tmpp.origin.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
	tmpp.origin.z = playerCharacter.pos.z;
	tmpp.radius = playerCharacter.physics.cyl.radius * 0.85f;
	float tmp = CheckAnythingInCylinder(&tmpp, inter.iobj[0], CFLAG_PLAYER | CFLAG_JUST_TEST);

	if (tmp <= 20.f)
		return TRUE;

	long hum = 0;

	for (float vv = 0; vv < 360.f; vv += 20.f)
	{
		tmpp.origin.x = playerCharacter.pos.x - EEsin(DEG2RAD(vv)) * 20.f;
		tmpp.origin.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
		tmpp.origin.z = playerCharacter.pos.z + EEcos(DEG2RAD(vv)) * 20.f;
		tmpp.radius = playerCharacter.physics.cyl.radius;
		float anything = CheckAnythingInCylinder(&tmpp, inter.iobj[0], CFLAG_JUST_TEST); //-cyl->origin.y;

		if (anything > 10)
		{
			hum = 1;
			break;
		}
	}

	if (!hum)
		return true;


	if (COLLIDED_CLIMB_POLY)
	{
		playerCharacter.climbing = 1;
		return true;
	}

	if ((tmp > 50.f)) 
	{
		return false;
	}

	return true;
}
extern float player_climb;
extern float PLAYER_CLIMB_THRESHOLD;
void PlayerMovementIterate(float DelatTime);

void ARX_PLAYER_Manage_Movement()
{
	// Is our player able to move ?
	if ((CINEMASCOPE)
	        ||	(BLOCK_PLAYER_CONTROLS)
	        ||	(inter.iobj[0] == NULL))
		return;

	float speedfactor;
	// Compute current player speedfactor
	speedfactor = inter.iobj[0]->basespeed + inter.iobj[0]->speed_modif;

	if (speedfactor < 0) speedfactor = 0;

	if (cur_mr == 3) speedfactor += 0.5f;

	if (cur_rf == 3) speedfactor += 1.5f;

	static float StoredTime = 0;
	// Compute time things
	float DeltaTime = StoredTime;

	if (Original_framedelay > 0)
		DeltaTime = StoredTime + (float)Original_framedelay * speedfactor; //FrameDiff;

	if (EDITMODE) DeltaTime = 25.f;

	if (playerCharacter.jumpphase)
	{
		while (DeltaTime > 25.f)
		{
			PlayerMovementIterate(DeltaTime);
			DeltaTime -= 25.f;
		}
	}
	else
	{
		PlayerMovementIterate(DeltaTime);
		DeltaTime = 0;
	}

	StoredTime = DeltaTime;
}

long JUMP_DIVIDE = 0;
void PlayerMovementIterate(float DeltaTime)
{
	// A jump is requested so let's go !
	if (REQUEST_JUMP)
	{
		if ((playerCharacter.Current_Movement & PLAYER_CROUCH) || (playerCharacter.physics.cyl.height > -170.f))
		{
			float old = playerCharacter.physics.cyl.height;
			playerCharacter.physics.cyl.height = PLAYER_BASE_HEIGHT;
			playerCharacter.physics.cyl.origin.x = playerCharacter.pos.x;
			playerCharacter.physics.cyl.origin.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
			playerCharacter.physics.cyl.origin.z = playerCharacter.pos.z;
			float anything = CheckAnythingInCylinder(&playerCharacter.physics.cyl, inter.iobj[0], CFLAG_JUST_TEST); //-cyl->origin.y;

			if (anything < 0.f)
			{
				playerCharacter.Current_Movement |= PLAYER_CROUCH;
				playerCharacter.physics.cyl.height = old;
				REQUEST_JUMP = 0;
			}
			else
			{
				bGCroucheToggle = false;
				playerCharacter.Current_Movement &= ~PLAYER_CROUCH;
				playerCharacter.physics.cyl.height = PLAYER_BASE_HEIGHT;
			}
		}

		if (!Valid_Jump_Pos())
			REQUEST_JUMP = 0;

		if (REQUEST_JUMP)
		{
			float t = (float)ARXTime - (float)REQUEST_JUMP;

			if ((t >= 0.f) && (t <= 350.f))
			{
				REQUEST_JUMP = 0;
				ARX_NPC_SpawnAudibleSound(&playerCharacter.pos, inter.iobj[0]);
				ARX_SPEECH_Launch_No_Unicode_Seek("[Player_Jump]", inter.iobj[0]);
				playerCharacter.onfirmground = 0;
				playerCharacter.jumpphase = 1;

			}
		}
	}


	if ((inter.iobj[0]->_npcdata->climb_count != 0.f) && (FrameDiff > 0))
	{
		inter.iobj[0]->_npcdata->climb_count -= MAX_ALLOWED_PER_SECOND * (float)FrameDiff * DIV10;

		if (inter.iobj[0]->_npcdata->climb_count < 0) inter.iobj[0]->_npcdata->climb_count = 0.f;
	}

	PLAYER_LEVITATE_HEIGHT = -220.f;

	float d = 0;

	if ((!EDITMODE) && (USE_PLAYERCOLLISIONS)) 
	{
		long levitate = 0;

		if (playerCharacter.climbing)
			levitate = 1;

		if (playerCharacter.levitate)
		{
			if (playerCharacter.physics.cyl.height != PLAYER_LEVITATE_HEIGHT)
			{
				float old = playerCharacter.physics.cyl.height;
				playerCharacter.physics.cyl.height = PLAYER_LEVITATE_HEIGHT;
				playerCharacter.physics.cyl.origin.x = playerCharacter.pos.x;
				playerCharacter.physics.cyl.origin.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
				playerCharacter.physics.cyl.origin.z = playerCharacter.pos.z;
				float anything = CheckAnythingInCylinder(&playerCharacter.physics.cyl, inter.iobj[0]); 

				if (anything < 0.f)
				{
					playerCharacter.physics.cyl.height = old;
					long num = ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_LEVITATE);

					if (num != -1)
					{
						spells[num].tolive = 0;
					}
				}
			}

			if (playerCharacter.physics.cyl.height == PLAYER_LEVITATE_HEIGHT)
			{
				levitate = 1;
				playerCharacter.climbing = 0;
				bGCroucheToggle = false;
				playerCharacter.Current_Movement &= ~PLAYER_CROUCH;
			}
		}
		else if (playerCharacter.physics.cyl.height == PLAYER_LEVITATE_HEIGHT)
		{
			playerCharacter.physics.cyl.height = PLAYER_BASE_HEIGHT;
		}

		if ((playerCharacter.jumpphase != 2) && !levitate)
		{
			playerCharacter.physics.cyl.origin.x = playerCharacter.pos.x;
			playerCharacter.physics.cyl.origin.y = playerCharacter.pos.y + 170.f;
			playerCharacter.physics.cyl.origin.z = playerCharacter.pos.z;
		}

		if (EEfabs(lastposy - playerCharacter.pos.y) < DeltaTime * DIV10) 
			TRUE_FIRM_GROUND = 1;
		else
			TRUE_FIRM_GROUND = 0;

		lastposy = playerCharacter.pos.y;
		DISABLE_JUMP = 0;
		float anything;
		EERIE_CYLINDER testcyl;
		memcpy(&testcyl, &playerCharacter.physics.cyl, sizeof(EERIE_CYLINDER));
		testcyl.origin.y += 3.f;
		ON_PLATFORM = 0;
		anything = CheckAnythingInCylinder(&testcyl, inter.iobj[0], 0);
		LAST_ON_PLATFORM = ON_PLATFORM;

		if (playerCharacter.jumpphase != 2)
		{
			if (anything >= 0.f)
			{
				TRUE_FIRM_GROUND = 0;
			}
			else
			{
				TRUE_FIRM_GROUND = 1;
				testcyl.radius -= 30.f;
				testcyl.origin.y -= 10.f;
				anything = CheckAnythingInCylinder(&testcyl, inter.iobj[0], 0);

				if (anything < 0.f)
				{
					DISABLE_JUMP = 1;
				}
			}
		}
		else
		{
			TRUE_FIRM_GROUND = 0;
			LAST_ON_PLATFORM = 0;
		}

		EERIE_CYLINDER cyl;
		cyl.origin.x = playerCharacter.pos.x;
		cyl.origin.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT + 1.f;
		cyl.origin.z = playerCharacter.pos.z;
		cyl.radius = playerCharacter.physics.cyl.radius;
		cyl.height = playerCharacter.physics.cyl.height;
		float anything2 = CheckAnythingInCylinder(&cyl, inter.iobj[0], CFLAG_JUST_TEST | CFLAG_PLAYER); //-cyl->origin.y;


		if ((anything2 > -5)
		        &&	(playerCharacter.physics.velocity.y > 15.f)
		        && !LAST_ON_PLATFORM
		        && !TRUE_FIRM_GROUND
		        && !playerCharacter.jumpphase
		        && !playerCharacter.levitate
		        && (anything > 80.f))
		{

			playerCharacter.jumpphase = 4;

			if (!playerCharacter.falling)
			{
				playerCharacter.falling = 1;
				ARX_PLAYER_StartFall();
			}
		}
		else if (!playerCharacter.falling)
			FALLING_TIME = 0;

		if ((playerCharacter.jumpphase)
		        &&	(playerCharacter.levitate))
		{
			playerCharacter.jumpphase = 0;
			playerCharacter.falling = 0;
			Falling_Height = playerCharacter.pos.y;
			FALLING_TIME = 0;
		}

		if ((!LAST_FIRM_GROUND) && (TRUE_FIRM_GROUND))
		{
			playerCharacter.jumpphase = 0;

			if ((FALLING_TIME > 0) && playerCharacter.falling)
			{
				playerCharacter.physics.velocity.x = 0.f;
				playerCharacter.physics.velocity.z = 0.f;
				playerCharacter.physics.forces.x = 0.f;
				playerCharacter.physics.forces.z = 0.f;

				playerCharacter.falling = 0;
		
				float fh = playerCharacter.pos.y - Falling_Height;

				if (fh > 400.f) 
				{
					float dmg = (fh - 400.f) * DIV15;

					if (dmg > 0.f)
					{
						Falling_Height = playerCharacter.pos.y;
						FALLING_TIME = 0;

						ARX_DAMAGES_DamagePlayer(dmg, 0, -1, &playerCharacter.pos);
						ARX_DAMAGES_DamagePlayerEquipment(dmg);
					}
				}
			}
		}

		LAST_FIRM_GROUND = TRUE_FIRM_GROUND;

		playerCharacter.onfirmground = TRUE_FIRM_GROUND;

		if (playerCharacter.onfirmground && !playerCharacter.falling)
			FALLING_TIME = 0;

		// Apply Player Impulse Force
		EERIE_3D mv;
		float TheoricalMove = 230;
		long time = 1000;

		float jump_mul = 1.f;

		if (LAST_JUMP_ENDTIME + 600 > ARXTime)
		{
			JUMP_DIVIDE = 1;

			if (LAST_JUMP_ENDTIME + 300 > ARXTime)
			{
				jump_mul = 0.5f;
			}
			else
			{
				jump_mul = 0.5f;
				jump_mul += (float)(LAST_JUMP_ENDTIME + 300 - ARXTime) * DIV300;

				if (jump_mul > 1.f) jump_mul = 1.f;
			}
		}

		if (inter.iobj[0]->animlayer[0].cur_anim)
		{
			GetAnimTotalTranslate(inter.iobj[0]->animlayer[0].cur_anim, inter.iobj[0]->animlayer[0].altidx_cur, &mv);
			TheoricalMove = TRUEVector_Magnitude(&mv);

			F2L(inter.iobj[0]->animlayer[0].cur_anim->anims[inter.iobj[0]->animlayer[0].altidx_cur]->anim_time, &time);

			if ((levitate) && (!playerCharacter.climbing))
			{
				TheoricalMove = 70;
				time = 1000;
			}

			if (playerCharacter.jumpphase)
			{
				TheoricalMove = 10;

				if (playerCharacter.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				{
					TheoricalMove = 420;

					if (playerCharacter.Current_Movement & PLAYER_MOVE_STRAFE_LEFT)
						TheoricalMove = 420; 

					if (playerCharacter.Current_Movement & PLAYER_MOVE_STRAFE_RIGHT)
						TheoricalMove = 420; 
				}
				else if (playerCharacter.Current_Movement & PLAYER_MOVE_STRAFE_LEFT)
					TheoricalMove = 140.f; 
				else if (playerCharacter.Current_Movement & PLAYER_MOVE_STRAFE_RIGHT)
					TheoricalMove = 140.f; 

				if (playerCharacter.Current_Movement & PLAYER_MOVE_WALK_BACKWARD)
					TheoricalMove = 40;

				time = 1000;
			}
		}
		else
		{
			TheoricalMove = 100;
			time = 1000;
		}

		TheoricalMove *= jump_mul; 
		
		float mval = TheoricalMove / time * DeltaTime; 

		if (playerCharacter.jumpphase == 2)
		{
			moveto.y = playerCharacter.pos.y; 
			playerCharacter.physics.velocity.y = 0;
		}

		EERIE_3D mv2;
		mv2.x = moveto.x - playerCharacter.pos.x;
		mv2.y = moveto.y - playerCharacter.pos.y;
		mv2.z = moveto.z - playerCharacter.pos.z;

		if (playerCharacter.climbing)
		{
			playerCharacter.physics.velocity.y *= DIV2;
		}

		if ((mv2.x == 0) && (mv2.y == 0) && (mv2.z == 0))
		{
		}
		else
		{
			float tt = 1.f / TRUEVector_Magnitude(&mv2);
			tt *= mval * DIV80;

			mv2.x = mv2.x * tt;
			mv2.y = mv2.y * tt;
			mv2.z = mv2.z * tt;
		}

		if (playerCharacter.climbing)
		{
			playerCharacter.physics.velocity.x = 0;
			playerCharacter.physics.velocity.z = 0;

			if (playerCharacter.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
			{
				moveto.x = playerCharacter.pos.x;
				moveto.z = playerCharacter.pos.z;
			}

			if (playerCharacter.Current_Movement & PLAYER_MOVE_WALK_BACKWARD)
			{
				mv2.x = 0;
				mv2.z = 0;
				moveto.x = playerCharacter.pos.x;
				moveto.z = playerCharacter.pos.z;
			}

		}

		playerCharacter.physics.forces.x += mv2.x;
		playerCharacter.physics.forces.y += mv2.y;
		playerCharacter.physics.forces.z += mv2.z;

		EERIE_3D modifplayermove;
		Vector_Init(&modifplayermove);

		// No Vertical Interpolation
		if (playerCharacter.jumpphase)
			inter.iobj[0]->_npcdata->vvpos = -99999.f;

		// Apply Gravity force if not LEVITATING or JUMPING
		if ((!levitate) && (playerCharacter.jumpphase != 2) && !LAST_ON_PLATFORM)
		{
			if (playerCharacter.falling)
				playerCharacter.physics.forces.y += JUMP_GRAVITY;
			else
				playerCharacter.physics.forces.y += WORLD_GRAVITY;

 
			float dist;
			EERIE_3D mod_vect;
			long mod_vect_count = -1;
			Vector_Init(&mod_vect);

			// Check for LAVA Damage !!!
			float epcentery;
			EERIEPOLY * ep = CheckInPoly(playerCharacter.pos.x, playerCharacter.pos.y + 150.f, playerCharacter.pos.z, &epcentery);

			if (ep)
			{
				dist = inter.iobj[0]->pos.y - epcentery;

				if ((ep->type & POLY_LAVA) && (EEfabs(epcentery - (playerCharacter.pos.y - PLAYER_BASE_HEIGHT)) < 30))
				{
					float mul = 1.f - (EEfabs(epcentery - (playerCharacter.pos.y - PLAYER_BASE_HEIGHT)) * DIV30);
#define LAVA_DAMAGE 10.f
					float damages = LAVA_DAMAGE * FrameDiff * DIV100 * mul;
					damages = ARX_SPELLS_ApplyFireProtection(inter.iobj[0], damages);

					ARX_DAMAGES_DamagePlayer(damages, DAMAGE_TYPE_FIRE, 0, NULL);
					ARX_DAMAGES_DamagePlayerEquipment(damages);
					EERIE_3D pos;
					pos.x = playerCharacter.pos.x;
					pos.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
					pos.z = playerCharacter.pos.z;
					ARX_PARTICLES_Spawn_Lava_Burn(&pos, 1.f, inter.iobj[0]);
				}
			}

			LAST_VECT_COUNT = mod_vect_count;
		}

		// Apply Velocity Damping (Natural Velocity Attenuation. Stands for friction)
		float dampen = 1.f - (0.009f * DeltaTime); 

		if (dampen < 0.001f) dampen = 0.f; 

		playerCharacter.physics.velocity.x *= dampen;
		playerCharacter.physics.velocity.z *= dampen;

		if (EEfabs(playerCharacter.physics.velocity.x) < 0.001f)
			playerCharacter.physics.velocity.x = 0;

		if (EEfabs(playerCharacter.physics.velocity.z) < 0.001f)
			playerCharacter.physics.velocity.z = 0;

		// Apply Attraction
		EERIE_3D attraction;
		ARX_SPECIAL_ATTRACTORS_ComputeForIO(inter.iobj[0], &attraction);
		playerCharacter.physics.forces.x += attraction.x;
		playerCharacter.physics.forces.y += attraction.y;
		playerCharacter.physics.forces.z += attraction.z;

		// Apply Push Player Force
		playerCharacter.physics.forces.x += PUSH_PLAYER_FORCE.x;
		playerCharacter.physics.forces.y += PUSH_PLAYER_FORCE.y;
		playerCharacter.physics.forces.z += PUSH_PLAYER_FORCE.z;
		PUSH_PLAYER_FORCE.x = 0;
		PUSH_PLAYER_FORCE.y = 0;
		PUSH_PLAYER_FORCE.z = 0;

		// Apply Forces To Velocity
		playerCharacter.physics.velocity.x += playerCharacter.physics.forces.x * DeltaTime;
		playerCharacter.physics.velocity.y += playerCharacter.physics.forces.y * DeltaTime;
		playerCharacter.physics.velocity.z += playerCharacter.physics.forces.z * DeltaTime;

		// Apply Climbing Velocity
		if (playerCharacter.climbing)
		{
			if (playerCharacter.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
			{
				playerCharacter.physics.velocity.y = -0.2f * DeltaTime;
			}

			if (playerCharacter.Current_Movement & PLAYER_MOVE_WALK_BACKWARD)
			{
				playerCharacter.physics.velocity.y = 0.2f * DeltaTime;
			}

		}

		// Removes Y Velocity if onfirmground...
		if ((playerCharacter.onfirmground == 1) && (!playerCharacter.climbing))
			playerCharacter.physics.velocity.y = 0.f;

		float posy;
		EERIEPOLY * ep = CheckInPolyPrecis(playerCharacter.pos.x, playerCharacter.pos.y, playerCharacter.pos.z, &posy);

		if (ep == NULL)
		{
			playerCharacter.physics.velocity.y = 0;
		}
		else if (!playerCharacter.climbing)
		{
			
			if (playerCharacter.pos.y >= posy) playerCharacter.physics.velocity.y = 0;
			
		}

		// Reset Forces
		memset(&playerCharacter.physics.forces, 0, sizeof(EERIE_3D));

		// Check if player is already on firm ground AND not moving
		if ((EEfabs(playerCharacter.physics.velocity.x) < 0.001f) &&
		        (EEfabs(playerCharacter.physics.velocity.z) < 0.001f) && (playerCharacter.onfirmground == 1)
		        && (playerCharacter.jumpphase == 0))
		{
			moveto.x = playerCharacter.pos.x;
			moveto.y = playerCharacter.pos.y;
			moveto.z = playerCharacter.pos.z;
			goto lasuite;
		}
		else // Need to apply some physics/collision tests
		{
			playerCharacter.physics.startpos.x = playerCharacter.physics.cyl.origin.x = playerCharacter.pos.x;
			playerCharacter.physics.startpos.y = playerCharacter.physics.cyl.origin.y = playerCharacter.pos.y - PLAYER_BASE_HEIGHT;
			playerCharacter.physics.startpos.z = playerCharacter.physics.cyl.origin.z = playerCharacter.pos.z;
			playerCharacter.physics.targetpos.x = playerCharacter.physics.startpos.x + playerCharacter.physics.velocity.x + modifplayermove.x * DeltaTime;
			playerCharacter.physics.targetpos.y = playerCharacter.physics.startpos.y + playerCharacter.physics.velocity.y + modifplayermove.y * DeltaTime;
			playerCharacter.physics.targetpos.z = playerCharacter.physics.startpos.z + playerCharacter.physics.velocity.z + modifplayermove.z * DeltaTime;

			// Jump Impulse
			if (playerCharacter.jumpphase == 2)
			{
				if (playerCharacter.jumplastposition == -1.f)
				{
					playerCharacter.jumplastposition = 0;
					playerCharacter.jumpstarttime = ARXTimeUL(); 
				}

				float jump_up_time	=	200.f;
				float jump_up_height =	130.f; 
				long timee			=	lARXTime;
				float offset_time	=	(float)timee - (float)playerCharacter.jumpstarttime;
				float divider		=	1.f / jump_up_time;
				float position		=	(float)offset_time * divider;

				if (position > 1.f) position = 1.f;

				if (position < 0.f) position = 0.f;

				float p1 = position;
				float p2 = playerCharacter.jumplastposition;
				playerCharacter.physics.targetpos.y -= (p1 - p2) * jump_up_height;
				Full_Jump_Height += (p1 - p2) * jump_up_height;
				playerCharacter.jumplastposition = position;
				levitate = 0;
			}

			BOOL test;
			APPLY_PUSH = 1;
			float PLAYER_CYLINDER_STEP = 40.f;

			if (playerCharacter.climbing)
			{
				test = ARX_COLLISION_Move_Cylinder(&playerCharacter.physics, inter.iobj[0], PLAYER_CYLINDER_STEP, CFLAG_EASY_SLIDING | CFLAG_CLIMBING | CFLAG_PLAYER);

				if (!COLLIDED_CLIMB_POLY)
					playerCharacter.climbing = 0;
			}
			else
			{
 
				test = ARX_COLLISION_Move_Cylinder(&playerCharacter.physics, inter.iobj[0], PLAYER_CYLINDER_STEP, levitate | CFLAG_EASY_SLIDING | CFLAG_PLAYER);

				if ((!test)
				        &&	((!LAST_FIRM_GROUND) && (!TRUE_FIRM_GROUND)))
				{
					playerCharacter.physics.velocity.x = 0.f;
					playerCharacter.physics.velocity.z = 0.f;
					playerCharacter.physics.forces.x = 0.f;
					playerCharacter.physics.forces.z = 0.f;

					if ((FALLING_TIME > 0) && playerCharacter.falling)
					{
						float fh = playerCharacter.pos.y - Falling_Height;

						if (fh > 400.f) 
						{
							float dmg = (fh - 400.f) * DIV15;

							if (dmg > 0.f)
							{
								Falling_Height = (playerCharacter.pos.y + Falling_Height * 2) * DIV3;
								ARX_DAMAGES_DamagePlayer(dmg, 0, -1, &playerCharacter.pos);
								ARX_DAMAGES_DamagePlayerEquipment(dmg);
							}
						}
					}
				}

				if ((test == FALSE) && (playerCharacter.jumpphase > 0))
				{

					playerCharacter.physics.startpos.x = playerCharacter.physics.cyl.origin.x = playerCharacter.pos.x;
					playerCharacter.physics.startpos.z = playerCharacter.physics.cyl.origin.z = playerCharacter.pos.z;
					playerCharacter.physics.targetpos.x = playerCharacter.physics.startpos.x;
					playerCharacter.physics.targetpos.z = playerCharacter.physics.startpos.z;

					if (playerCharacter.physics.targetpos.y != playerCharacter.physics.startpos.y)
					{
						test = ARX_COLLISION_Move_Cylinder(&playerCharacter.physics, inter.iobj[0], PLAYER_CYLINDER_STEP, levitate | CFLAG_EASY_SLIDING | CFLAG_PLAYER);
						inter.iobj[0]->_npcdata->vvpos = -99999.f;
					}
				}
			}

			//LAST_ON_PLATFORM=ON_PLATFORM;
			if (COLLIDED_CLIMB_POLY)
				playerCharacter.climbing = 1;

			if (playerCharacter.climbing)
			{
				if ((playerCharacter.Current_Movement != 0) && (playerCharacter.Current_Movement != PLAYER_ROTATE)
				        && !(playerCharacter.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				        && !(playerCharacter.Current_Movement & PLAYER_MOVE_WALK_BACKWARD))
					playerCharacter.climbing = 0;

				if ((playerCharacter.Current_Movement & PLAYER_MOVE_WALK_BACKWARD)  && !test)
					playerCharacter.climbing = 0;

				if (playerCharacter.climbing)
				{
					playerCharacter.jumpphase = 0;
					playerCharacter.falling = 0;
					FALLING_TIME = 0;
					Falling_Height = playerCharacter.pos.y;
				}
			}

			if (playerCharacter.jumpphase == 2)
			{
				playerCharacter.climbing = 0;
			}

			APPLY_PUSH = 0;
			moveto.x = playerCharacter.physics.cyl.origin.x;
			moveto.y = playerCharacter.physics.cyl.origin.y + PLAYER_BASE_HEIGHT;
			moveto.z = playerCharacter.physics.cyl.origin.z;
		
			d = (float)(TRUEEEDistance3D(&playerCharacter.pos, &moveto));
		}
	}
	else
	{
	
		if (!EDITMODE)
		{
			EERIE_3D vect;
			vect.x = moveto.x - playerCharacter.pos.x;
			vect.y = moveto.y - playerCharacter.pos.y;
			vect.z = moveto.z - playerCharacter.pos.z;
			float divv = TRUEVector_Magnitude(&vect);

			if (divv > 0.f)
			{
				float mul = (float)FrameDiff * DIV1000 * 200.f;
				divv = mul / divv;
				vect.x *= divv;
				vect.y *= divv;
				vect.z *= divv;
				moveto.x = playerCharacter.pos.x + vect.x;
				moveto.y = playerCharacter.pos.y + vect.y;
				moveto.z = playerCharacter.pos.z + vect.z;
			}
		}

		playerCharacter.onfirmground = 0;
	}


	if (Vector_Compare(&playerCharacter.pos, &moveto)) d = 0.f;

	// Emit Stepsound
	if ((USE_PLAYERCOLLISIONS) && (!EDITMODE))
	{
		if (playerCharacter.Current_Movement & PLAYER_CROUCH)
		{
			d *= 2.f;
		}

		currentdistance += d;

		if ((!playerCharacter.jumpphase)
		        &&	(!playerCharacter.falling)
		        &&	(currentdistance >= STEP_DISTANCE))
			ARX_PLAYER_MakeStepNoise();
	}

	// Finally update player pos !
	Vector_Copy(&playerCharacter.pos, &moveto);

lasuite:
	;

	// Get Player position color
	unsigned long col = GetColorz(playerCharacter.pos.x, playerCharacter.pos.y + 90, playerCharacter.pos.z);
	playerCharacter.grnd_color = (float)(long)(col & 255);
	playerCharacter.grnd_color += (float)(long)((col >> 8) & 255);
	playerCharacter.grnd_color += (float)(long)((col >> 16) & 255);
	playerCharacter.grnd_color *= DIV3;
	playerCharacter.grnd_color -= 15.f; 
	if (CURRENT_PLAYER_COLOR < playerCharacter.grnd_color)
	{
		CURRENT_PLAYER_COLOR += FrameDiff * DIV8;
		CURRENT_PLAYER_COLOR = __min(CURRENT_PLAYER_COLOR, playerCharacter.grnd_color);
	}

	if (CURRENT_PLAYER_COLOR > playerCharacter.grnd_color)
	{
		CURRENT_PLAYER_COLOR -= FrameDiff * DIV4;
		CURRENT_PLAYER_COLOR = __max(CURRENT_PLAYER_COLOR, playerCharacter.grnd_color);
	}

	if (InventoryDir != 0)
	{
		if ((playerCharacter.Interface & INTER_COMBATMODE) || (playerCharacter.doingmagic >= 2) || (InventoryDir == -1))
		{
			if (InventoryX > -160)
				InventoryX -= INTERFACE_RATIO(FrameDiff * DIV3);
		}
		else
		{
			if (InventoryX < 0)
				InventoryX += InventoryDir * INTERFACE_RATIO(FrameDiff * DIV3);
		}

		if (InventoryX <= -160)
		{
			InventoryX = -160;
			InventoryDir = 0;

			if (playerCharacter.Interface & INTER_STEAL || ioSteal)
			{
				SendIOScriptEvent(ioSteal, SM_STEAL, "OFF");
				playerCharacter.Interface &= ~INTER_STEAL;
				ioSteal = NULL;
			}

			SecondaryInventory = NULL;
			TSecondaryInventory = NULL;
			InventoryDir = 0;
		}
		else if (InventoryX >= 0)
		{
			InventoryX = 0;
			InventoryDir = 0;
		}
	}
}
//******************************************************************************
// Manage Player Death Visual
//******************************************************************************
void ARX_PLAYER_Manage_Death()
{
	PLAYER_PARALYSED = 0;
	float ratio = (float)(DeadTime - 2000) * DIV5000;

	if (ratio >= 1.f) ratio = 1.f;

	if (ratio == 1.f)
	{
		ARX_MENU_Launch(GDevice);
		DeadTime = 0;
	}

	{
		g_pRenderApp->renderer->SetAlphaBlend(true);
		g_pRenderApp->renderer->SetBlendFunc(EERIEBlendType::Zero, EERIEBlendType::OneMinusSrcColor);
		g_pRenderApp->renderer->DrawQuad(0.f, 0.f, ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX), ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZY), 0.000091f,
		                NULL, 0, _EERIERGB(ratio));

	}
}
//******************************************************************************
// Specific for color checks
//******************************************************************************
float GetPlayerStealth()
{
	return 15 + playerCharacter.Full_Skill_Stealth * DIV10;
}

//******************************************************************************
// Teleport player to any poly...
//******************************************************************************
void ARX_PLAYER_GotoAnyPoly()
{
	for (long j = 0; j < ACTIVEBKG->Zsize; j++)
	{
		for (long i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			EERIE_BKG_INFO * eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			if (eg->nbpoly)
			{
				playerCharacter.pos.x = moveto.x = eg->polydata[0].center.x;
				playerCharacter.pos.y = moveto.y = eg->polydata[0].center.y + PLAYER_BASE_HEIGHT;
				playerCharacter.pos.z = moveto.z = eg->polydata[0].center.z;
			}
		}
	}
}
//******************************************************************************
// Force Player to standard stance... (Need some improvements...)
//******************************************************************************
void ARX_PLAYER_PutPlayerInNormalStance(long val)
{
	if (playerCharacter.Current_Movement & PLAYER_CROUCH)
		playerCharacter.Current_Movement &= ~PLAYER_CROUCH;

	playerCharacter.Current_Movement = 0;
	ARX_PLAYER_RectifyPosition();

	if ((playerCharacter.jumpphase) || (playerCharacter.falling))
	{
		playerCharacter.physics.cyl.origin.x = playerCharacter.pos.x;
		playerCharacter.physics.cyl.origin.y = playerCharacter.pos.y + 170.f;

		playerCharacter.physics.cyl.origin.z = playerCharacter.pos.z;
		IO_PHYSICS phys;
		memcpy(&phys, &playerCharacter.physics, sizeof(IO_PHYSICS));
		AttemptValidCylinderPos(&phys.cyl, inter.iobj[0], CFLAG_RETURN_HEIGHT);
		playerCharacter.pos.y = phys.cyl.origin.y - 170.f;
		playerCharacter.jumpphase = 0;
		playerCharacter.falling = 0;
	}

	if (playerCharacter.Interface & INTER_COMBATMODE)
	{
		playerCharacter.Interface &= ~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}


	ARX_SOUND_Stop(SND_MAGIC_DRAW);

	if (!val)
		for (long i = 0; i < MAX_SPELLS; i++)
		{
			if ((spells[i].exist)
			        && ((spells[i].caster == 0) || (spells[i].target == 0)))
			{
				switch (spells[i].type)
				{
					case SPELL_MAGIC_SIGHT:
					case SPELL_LEVITATE:
					case SPELL_SPEED:
					case SPELL_FLYING_EYE:
						spells[i].tolive = 0;
						break;
				}
			}
		}

}

//******************************************************************************
// Add gold to player purse
//******************************************************************************
void ARX_PLAYER_AddGold(long _lValue)
{
	playerCharacter.gold += _lValue;
	bGoldHalo = true;
	ulGoldHaloTime = 0;
}
extern long GAME_EDITOR;
void ARX_PLAYER_Start_New_Quest()
{

	SKIN_MOD = 0;
	QUICK_MOD = 0;
	EERIE_PATHFINDER_Clear();
	EERIE_PATHFINDER_Release();
	ARX_PLAYER_MakeFreshHero();
	CURRENT_TORCH = NULL;
	FreeAllInter();
	SecondaryInventory = NULL;
	TSecondaryInventory = NULL;
	ARX_EQUIPMENT_UnEquipAllPlayer();
	//Empty Current Game Directory to restart a new game...
	ARX_CHANGELEVEL_MakePath();
	KillAllDirectory(CurGamePath);
	CreateDirectory(CurGamePath, NULL);
	inter.iobj[0]->halo.flags = 0;
	
}

//-----------------------------------------------------------------------------
void ARX_PLAYER_AddBag()
{
	playerCharacter.bag ++;

	if (playerCharacter.bag > 3)
	{
		playerCharacter.bag = 3;
	}
}

//-----------------------------------------------------------------------------
bool ARX_PLAYER_CanStealItem(INTERACTIVE_OBJ * _io)
{
	
	if (_io->_itemdata->stealvalue > 0)
		if ((playerCharacter.Full_Skill_Stealth >= _io->_itemdata->stealvalue)   
		        &&	(_io->_itemdata->stealvalue < 100.f))
		{
			return true;
		}

	return false;
}
void ARX_PLAYER_Rune_Add_All()
{
	ARX_Player_Rune_Add(FLAG_AAM);
	ARX_Player_Rune_Add(FLAG_CETRIUS);
	ARX_Player_Rune_Add(FLAG_COMUNICATUM);
	ARX_Player_Rune_Add(FLAG_COSUM);
	ARX_Player_Rune_Add(FLAG_FOLGORA);
	ARX_Player_Rune_Add(FLAG_FRIDD);
	ARX_Player_Rune_Add(FLAG_KAOM);
	ARX_Player_Rune_Add(FLAG_MEGA);
	ARX_Player_Rune_Add(FLAG_MORTE);
	ARX_Player_Rune_Add(FLAG_MOVIS);
	ARX_Player_Rune_Add(FLAG_NHI);
	ARX_Player_Rune_Add(FLAG_RHAA);
	ARX_Player_Rune_Add(FLAG_SPACIUM);
	ARX_Player_Rune_Add(FLAG_STREGUM);
	ARX_Player_Rune_Add(FLAG_TAAR);
	ARX_Player_Rune_Add(FLAG_TEMPUS);
	ARX_Player_Rune_Add(FLAG_TERA);
	ARX_Player_Rune_Add(FLAG_VISTA);
	ARX_Player_Rune_Add(FLAG_VITAE);
	ARX_Player_Rune_Add(FLAG_YOK);
}

extern unsigned long LAST_PRECAST_TIME;
extern long sp_wep;
extern long TOTAL_BODY_CHUNKS_COUNT;
extern long WILL_QUICKLOAD, WILL_QUICKSAVE;
extern long GLOBAL_Player_Room;
extern long cur_mx, cur_pom;
extern long sp_arm, cur_arm;
extern float sp_max_start; 	

void ARX_PLAYER_Invulnerability(long flag)
{
	if (flag)
		playerCharacter.playerflags |= PLAYERFLAGS_INVULNERABILITY;
	else
		playerCharacter.playerflags &= ~PLAYERFLAGS_INVULNERABILITY;
}
extern INTERACTIVE_OBJ * FlyingOverIO;
extern long cur_sm;
extern void ClearDynLights();
extern void ClearTileLights();

void ARX_GAME_Reset(long type)
{
	ARX_SPELLS_CancelAll();

	LAST_JUMP_ENDTIME = 0;
	FlyingOverIO = NULL;
	ARX_MAPMARKER_Init();
	ClearDynLights();

	if (!DONT_ERASE_PLAYER)
	{
		inter.iobj[0]->halo.flags = 0;
	}

	inter.iobj[0]->GameFlags &= ~GFLAG_INVISIBILITY;
	ARX_PLAYER_Invulnerability(0);
	GLOBAL_Player_Room = -1;
	PLAYER_PARALYSED = 0;

	ARX_PLAYER_Reset_Fall();

	playerCharacter.levitate = 0;
	Project.telekinesis = 0;
	playerCharacter.onfirmground = 0;
	TRUE_FIRM_GROUND = 0;
	sp_max_start = 0;
	lastposy = -99999999999.f;

	ioSteal = NULL;

	WILL_QUICKLOAD = 0;
	WILL_QUICKSAVE = 0;
	GLOBAL_SLOWDOWN = 1.f;

	PrecalcIOLighting(NULL, 0, 1);

	sp_arm = 0;
	cur_arm = 0;
	cur_sm = 0;
	sp_wep = 0;
	sp_max = 0;
	cur_mx = 0;
	cur_pom = 0;
	cur_rf = 0;
	cur_mr = 0; 


	if (inter.iobj[0]) inter.iobj[0]->spellcast_data.castingspell = -1;

	LAST_PRECAST_TIME = 0;

	ARX_INTERFACE_NoteClear();
	playerCharacter.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;

	// Interactive DynData
	ARX_INTERACTIVE_ClearAllDynData();

	// PolyBooms
	ARX_BOOMS_ClearAllPolyBooms();

	// Magical Flares
	ARX_MAGICAL_FLARES_KillAll();

	// Thrown Objects
	ARX_THROWN_OBJECT_KillAll();

	// Pathfinder
	EERIE_PATHFINDER_Clear();

	// Sound
	if (!(type & 1))
	{
		ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
		ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
		ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
	}

	// Damages
	ARX_DAMAGES_SCREEN_SPLATS_Init();
	ARX_DAMAGE_Reset_Blood_Info();
	ARX_DAMAGES_Reset();

	// Scripts
	ARX_SCRIPT_Timer_ClearAll();
	ARX_SCRIPT_EventStackClear();
	ARX_SCRIPT_ResetAll(0);

	// Conversations
	ARX_CONVERSATION_Reset();
	ARX_CONVERSATION = 0;

	// Speech Things
	REQUEST_SPEECH_SKIP = 0;
	ARX_SPEECH_ClearAll(); // to remove later...
	ARX_SPEECH_Reset();

	// Spells
	ARX_SPELLS_Precast_Reset();
	ARX_SPELLS_CancelSpellTarget();

	ARX_SPELLS_ClearAll();
	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_SPELLS_ResetRecognition();

	// Particles
	ARX_PARTICLES_ClearAll();
	pParticleManager->Clear();

	// Fogs
	ARX_FOGS_TimeReset();
	ARX_FOGS_Render(1);

	// Anchors
	ANCHOR_BLOCK_Clear();

	// Attractors
	ARX_SPECIAL_ATTRACTORS_Reset();

	// Cinematics
	DANAE_KillCinematic();

	// Paths
	ARX_PATH_ClearAllControled();
	ARX_PATH_ClearAllUsePath();

	// Player Torch
	if (type & 1)
	{
		if (CURRENT_TORCH) ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
	}
	else
		CURRENT_TORCH = NULL;

	// Player Quests
	ARX_PLAYER_Quest_Init();

	// Player Keyring
	ARX_KEYRING_Init();

	// Player Init
	if (!DONT_ERASE_PLAYER)
	{
		ARX_MAPMARKER_Init();
		GLOBAL_MAGIC_MODE = 1;

		// Linked Objects
		if (!(type & 2))
		{
			UnlinkAllLinkedObjects();
			ARX_EQUIPMENT_UnEquipAllPlayer();
		}

		ARX_EQUIPMENT_ReleaseAll(inter.iobj[0]);

		ARX_PLAYER_InitPlayer();
		ARX_INTERACTIVE_RemoveGoreOnIO(inter.iobj[0]); 
		TRUE_PLAYER_MOUSELOOK_ON = 0;
		// Player Inventory
		CleanInventory();
	}

	// Misc Player Vars.
	ROTATE_START = 0;
	BLOCK_PLAYER_CONTROLS = 0;
	HERO_SHOW_1ST = -1;
	Vector_Init(&PUSH_PLAYER_FORCE);
	playerCharacter.jumplastposition = 0;
	playerCharacter.jumpstarttime = 0;
	playerCharacter.jumpphase = 0;
	playerCharacter.inzone = NULL;

	QuakeFx.intensity = 0.f;
	Project.improve = 0;

	if (eyeball.exist) eyeball.exist = -100;

	if ((inter.iobj) && (inter.nbmax > 0) && (inter.iobj[0]))
	{
		inter.iobj[0]->ouch_time = 0;
		inter.iobj[0]->invisibility = 0.f;
	}

	FADEDIR = 0;
	FADEDURATION = 0;
	FADESTART = 0;
	FADECOLOR.r = 0;
	FADECOLOR.b = 0;
	FADECOLOR.g = 0;

	// GLOBALMods
	ARX_GLOBALMODS_Reset();

	// Missiles
	ARX_MISSILES_ClearAll();

	// IO PDL
	TOTIOPDL = 0;

	// Interface
	ARX_INTERFACE_Reset();
	ARX_INTERFACE_NoteClear();
	Set_DragInter(NULL);
	SecondaryInventory = NULL;
	TSecondaryInventory = NULL;
	MasterCamera.exist = 0;
	CHANGE_LEVEL_ICON = -1;

	CAMERACONTROLLER = NULL;

	// Kill Script Loaded IO
	CleanScriptLoadedIO();

	// Localisation
	if (!FINAL_RELEASE)
		ARX_Localisation_Init();

	// ARX Debugger
	NEED_DEBUGGER_CLEAR = 1;

	//Body chunks count
	TOTAL_BODY_CHUNKS_COUNT = 0;

	// ARX Timer
	ARX_TIME_Init();

	ClearTileLights();
}

void ARX_PLAYER_Reset_Fall()
{
	FALLING_TIME = 0;
	Falling_Height = 50.f;
	playerCharacter.falling = 0;
}



float sp_max_y[64];
COLORREF sp_max_col[64];
char	sp_max_ch[64];
long sp_max_nb;
void Manage_sp_max()
{
	float v = ARXTime - sp_max_start;

	if ((sp_max_start != 0) && (v < 20000))
	{
		float modi = (20000 - v) * DIV2000 * DIV10;
		float sizX = 16;
		float px = (float)DANAECENTERX - (float)sp_max_nb * DIV2 * sizX;
		float py = (float)DANAECENTERY;

		for (long i = 0; i < sp_max_nb; i++)
		{
			float dx = px + sizX * (float)i;
			float dy = py + sp_max_y[i];
			sp_max_y[i] = EEsin(dx + (float)ARXTime * DIV100) * 30.f * modi;
			_TCHAR tex[8];
			_stprintf(tex, _T("%c"), sp_max_ch[i]);
			
			UNICODE_ARXDrawTextCenter(dx - 1, dy - 1, tex, 0, 0x00FF00FF, InBookFont);
			UNICODE_ARXDrawTextCenter(dx + 1, dy + 1, tex, 0, 0x00FF00FF, InBookFont);
			UNICODE_ARXDrawTextCenter(dx, dy, tex, sp_max_col[i], 0x00FF00FF, InBookFont);

		}
	}
}
