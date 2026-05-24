//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LNEXTBOTPATH_H
#define LNEXTBOTPATH_H
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

class CLuaPathCost : public IPathCost
{
public:
	CLuaPathCost( lua_State *L, INextBot *bot, int funcRef = LUA_NOREF );

	virtual float operator()( CNavArea *area, CNavArea *fromArea, const CNavLadder *ladder, const CFuncElevator *elevator, float length ) const;

private:
	lua_State *m_L;
	INextBot  *m_bot;
	int		   m_luaFuncRef;
};

/*
** access functions (stack -> C)
*/

PathFollower *luaL_checknextbotpath( lua_State *L, int narg );


/*
** push functions (C -> stack)
*/

void lua_pushnextbotpath( lua_State *L, PathFollower *pPath );


#endif // LNEXTBOTPATH_H
