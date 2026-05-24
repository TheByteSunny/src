//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "lc_nextbot.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_LuaNextBot, DT_LuaNextBot, CLuaNextBot )
    RecvPropInt( RECVINFO( m_fEffects ) ),
END_RECV_TABLE()

C_LuaNextBot::C_LuaNextBot()
{
}

C_LuaNextBot::~C_LuaNextBot()
{
}

void C_LuaNextBot::Spawn( void )
{
	BaseClass::Spawn();

	SetNextClientThink( CLIENT_THINK_NEVER );
}

void C_LuaNextBot::UpdateClientSideAnimation( void )
{
	BaseClass::UpdateClientSideAnimation();
}

ShadowType_t C_LuaNextBot::ShadowCastType( void )
{
	return BaseClass::ShadowCastType();
}
