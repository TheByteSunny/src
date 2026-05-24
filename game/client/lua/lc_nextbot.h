//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LC_NEXTBOT_H
#define LC_NEXTBOT_H
#ifdef _WIN32
#pragma once
#endif

#include "NextBot/C_NextBot.h"

class C_LuaNextBot : public C_NextBotCombatCharacter
{
public:
	DECLARE_CLASS( C_LuaNextBot, C_NextBotCombatCharacter );
	DECLARE_CLIENTCLASS();

	C_LuaNextBot();
	virtual ~C_LuaNextBot();

	virtual void		 Spawn( void );
	virtual void		 UpdateClientSideAnimation( void );
	virtual ShadowType_t ShadowCastType( void );

private:
	C_LuaNextBot( const C_LuaNextBot & ); // no copies
};

#endif // LC_NEXTBOT_H
