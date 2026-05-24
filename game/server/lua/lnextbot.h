//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LNEXTBOT_H
#define LNEXTBOT_H
#ifdef _WIN32
#pragma once
#endif

#include "NextBot/NextBotGroundLocomotion.h"
#include "NextBot/NextBotInterface.h"
#include "NextBot/NextBotBehavior.h"
#include "NextBot/Path/NextBotPathFollow.h"
#include "NextBot/Path/NextBotChasePath.h"
#include "NextBot/Path/NextBotPath.h"
#include "NextBot/NextBot.h"

class CLuaNextBotLocomotion;
class CLuaNextBotIntention;

class CLuaNextBot : public NextBotCombatCharacter
{
public:
	DECLARE_CLASS( CLuaNextBot, NextBotCombatCharacter );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CLuaNextBot();
	virtual ~CLuaNextBot();

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void UpdateOnRemove( void );

	virtual ILocomotion *GetLocomotionInterface( void ) const
	{
		return (ILocomotion *)m_locomotion;
	}

	virtual IIntention *GetIntentionInterface( void ) const
	{
		return (IIntention *)m_intention;
	}

	virtual void OnStuck( void );
	virtual void OnUnStuck( void );
	virtual void OnLeaveGround( CBaseEntity *ground );
	virtual void OnLandOnGround( CBaseEntity *ground );
	virtual int	 OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	bool CallLuaHook( const char *hookName );

	CLuaNextBotLocomotion *GetLuaLocomotion( void )
	{
		return m_locomotion;
	}

	int m_nLuaRef;

private:
	CLuaNextBotLocomotion *m_locomotion;
	CLuaNextBotIntention  *m_intention;
};

void RegisterScriptedNextbot( const char *className );

/*
** push functions (C -> stack)
*/

/*
** access functions (stack -> C)
*/

INextBot *luaL_checknextbot( lua_State *L, int narg );

#endif // LNEXTBOT_H