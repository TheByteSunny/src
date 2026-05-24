//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LNEXTBOTINTENTION_H
#define LNEXTBOTINTENTION_H
#ifdef _WIN32
#pragma once
#endif

#include "NextBot/NextBotGroundLocomotion.h"
#include "NextBot/NextBotInterface.h"
#include "NextBot/NextBotBehavior.h"
#include "NextBot/Path/NextBotPathFollow.h"
#include "NextBot/Path/NextBotPath.h"
#include "lnextbotbehaviour.h"

class CLuaNextBot;

class CLuaNextBotIntention : public IIntention
{
public:
	CLuaNextBotIntention( INextBot *bot );
	virtual ~CLuaNextBotIntention();

	virtual void Reset( void );
	virtual void Update( void );

	virtual QueryResultType IsHindrance( const INextBot *me, CBaseEntity *blocker ) const
	{
		return ANSWER_UNDEFINED;
	}

private:
	Behavior< CLuaNextBot > *m_behaviour;
};

#endif // LNEXTBOTINTENTION_H