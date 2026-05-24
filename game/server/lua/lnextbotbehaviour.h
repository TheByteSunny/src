//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LNEXTBOTBEHAVIOUR_H
#define LNEXTBOTBEHAVIOUR_H
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
#include "lnextbot.h"

class CLuaNextBotIntention;
class CLuaNextBot;

class CLuaBehaviour : public Action< CLuaNextBot >
{
public:
	virtual ActionResult< CLuaNextBot > OnStart( CLuaNextBot *me, Action< CLuaNextBot > *priorAction );
	virtual ActionResult< CLuaNextBot > Update( CLuaNextBot *me, float interval );

	virtual const char				   *GetName( void ) const
	{
		return "LuaBotBehaviour";
	}
};

#endif // LNEXTBOTBEHAVIOUR_H