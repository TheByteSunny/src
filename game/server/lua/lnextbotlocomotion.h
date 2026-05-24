//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LNEXTBOTLOCOMOTION_H
#define LNEXTBOTLOCOMOTION_H
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
#include "luamanager.h"

class CLuaNextBotLocomotion : public NextBotGroundLocomotion
{
	DECLARE_CLASS( CLuaNextBotLocomotion, NextBotGroundLocomotion );

public:
	CLuaNextBotLocomotion( INextBot *bot );

	virtual float GetStepHeight( void ) const
	{
		return m_flStepHeight;
	}

	virtual float GetMaxJumpHeight( void ) const
	{
		return m_flJumpHeight;
	}

	virtual float GetDeathDropHeight( void ) const
	{
		return m_flDeathDrop;
	}

	virtual float GetMaxAcceleration( void ) const
	{
		return m_flMaxAccel;
	}

	virtual float GetMaxDeceleration( void ) const
	{
		return m_flMaxDecel;
	}

	void SetStepHeight( float f );
	void SetJumpHeight( float f );
	void SetDeathDropHeight( float f );
	void SetDeceleration( float f );
	void SetMaxAcceleration( float f );

private:
	float m_flStepHeight;
	float m_flJumpHeight;
	float m_flDeathDrop;
	float m_flMaxAccel;
	float m_flMaxDecel;
};

/*
** access functions (stack -> C)
*/

CLuaNextBotLocomotion *luaL_checklocomotion( lua_State *L, int narg );

/*
** push functions (C -> stack)
*/

void			lua_pushlocomotion( lua_State *L, CLuaNextBotLocomotion *pLoco );


#endif // LNEXTBOTLOCOMOTION_H