//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "lnextbotlocomotion.h"
#include "lbaseentity_shared.h"
#include "mathlib/lvector.h"
#include "NextBot/NextBot.h"
#include "lnextbotlocomotion.h"
#include "lnextbotintention.h"
#include "lnextbot.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/

INextBot *luaL_checknextbot( lua_State *L, int narg )
{
	if ( lua_istable( L, narg ) )
	{
		lua_getfield( L, narg, "Entity" );
		if ( lua_isuserdata( L, -1 ) )
		{
			CBaseEntity *pEnt = luaL_checkentity( L, -1 );
			lua_pop( L, 1 );
			if ( pEnt )
			{
				INextBot *bot = dynamic_cast< INextBot * >( pEnt );
				if ( bot )
					return bot;
			}
		}
		else
			lua_pop( L, 1 );

		luaL_error( L, "table is not a NextBot entity" );
		return NULL;
	}

	CBaseEntity *pEnt = luaL_checkentity( L, narg );
	if ( !pEnt )
		luaL_error( L, "invalid entity for NextBot" );

	INextBot *bot = dynamic_cast< INextBot * >( pEnt );
	if ( !bot )
		luaL_error( L, "entity is not a NextBot" );

	return bot;
}

LINK_ENTITY_TO_CLASS( lua_nextbot, CLuaNextBot );

IMPLEMENT_SERVERCLASS_ST( CLuaNextBot, DT_LuaNextBot )
    SendPropInt( SENDINFO( m_fEffects ), EF_MAX_BITS, SPROP_UNSIGNED ),
END_SEND_TABLE()

BEGIN_DATADESC( CLuaNextBot )
END_DATADESC()

CLuaNextBot::CLuaNextBot()
{
	m_locomotion = NULL;
	m_intention = NULL;
	m_nLuaRef = LUA_NOREF;
}

CLuaNextBot::~CLuaNextBot()
{
	if ( m_nLuaRef != LUA_NOREF )
	{
		luaL_unref( L, LUA_REGISTRYINDEX, m_nLuaRef );
		m_nLuaRef = LUA_NOREF;
	}
}

void CLuaNextBot::Spawn( void )
{
	Precache();
	m_locomotion = new CLuaNextBotLocomotion( this );
	m_intention = new CLuaNextBotIntention( this );
	BaseClass::Spawn();
	SetModel( "models/error.mdl" );

	AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

	SetMoveType( MOVETYPE_CUSTOM );
	SetSolid( SOLID_BBOX );
	SetCollisionGroup( COLLISION_GROUP_NPC );

	// Look for the defined tables. Please, Moon, add the damn support to the base.
	lua_getglobal( L, "entity" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "get" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_remove( L, -2 );
			lua_pushstring( L, GetClassname() );
			luasrc_pcall( L, 1, 1, 0 );
			if ( lua_istable( L, -1 ) )
				m_nLuaRef = luaL_ref( L, LUA_REGISTRYINDEX );
			else
				lua_pop( L, 1 );
		}
		else
			lua_pop( L, 2 );
	}
	else
		lua_pop( L, 1 );

	// expose shit
	if ( m_nLuaRef != LUA_NOREF )
	{
		lua_rawgeti( L, LUA_REGISTRYINDEX, m_nLuaRef );

		lua_pushlocomotion( L, m_locomotion );
		lua_setfield( L, -2, "loco" );

		lua_pushentity( L, this );
		lua_setfield( L, -2, "Entity" );

		lua_newtable( L );
		lua_pushentity( L, this );
		lua_pushcclosure(
			L,
			[]( lua_State *L ) -> int
			{
				lua_pushvalue( L, lua_upvalueindex( 1 ) );
				lua_pushvalue( L, 2 );
				lua_gettable( L, -2 );
				lua_remove( L, -2 );
				return 1;
			},
			1 );
		lua_setfield( L, -2, "__index" );
		lua_setmetatable( L, -2 );

		lua_pop( L, 1 );
	}

	CallLuaHook( "Initialize" );
}

void CLuaNextBot::Precache( void )
{
	BaseClass::Precache();
	//PrecacheModel( "models/error.mdl" );
}

void CLuaNextBot::UpdateOnRemove( void )
{
	CallLuaHook( "OnRemove" );

	if ( m_locomotion )
	{
		delete m_locomotion;
		m_locomotion = NULL;
	}

	if ( m_intention )
	{
		delete m_intention;
		m_intention = NULL;
	}

	BaseClass::UpdateOnRemove();
}

bool CLuaNextBot::CallLuaHook( const char *hookName )
{
	if ( !g_bLuaInitialized || m_nLuaRef == LUA_NOREF )
		return false;

	lua_rawgeti( L, LUA_REGISTRYINDEX, m_nLuaRef );
	if ( !lua_istable( L, -1 ) )
	{
		lua_pop( L, 1 );
		return false;
	}

	lua_getfield( L, -1, hookName );
	if ( !lua_isfunction( L, -1 ) )
	{
		lua_pop( L, 2 );
		return false;
	}

	lua_pushvalue( L, -2 );
	lua_remove( L, -3 );

	luasrc_pcall( L, 1, 0, 0 );
	return true;
}

void CLuaNextBot::OnStuck( void )
{
	CallLuaHook( "OnStuck" );
}

void CLuaNextBot::OnUnStuck( void )
{
	CallLuaHook( "OnUnStuck" );
}

void CLuaNextBot::OnLeaveGround( CBaseEntity * )
{
	CallLuaHook( "OnLeaveGround" );
}

void CLuaNextBot::OnLandOnGround( CBaseEntity * )
{
	CallLuaHook( "OnLandOnGround" );
}

int CLuaNextBot::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CallLuaHook( "OnInjured" );
	return BaseClass::OnTakeDamage_Alive( info );
}

void CLuaNextBot::Event_Killed( const CTakeDamageInfo &info )
{
	CallLuaHook( "OnKilled" );
	BaseClass::Event_Killed( info );
}

static int nextbot_GetRangeTo( lua_State *L )
{
	CBaseEntity *pEnt = luaL_checkentity( L, 1 );
	if ( !pEnt )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	CLuaNextBot *bot = dynamic_cast< CLuaNextBot * >( pEnt );
	if ( !bot )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	if ( lua_isuserdata( L, 2 ) )
		lua_pushnumber( L, bot->GetRangeTo( luaL_checkentity( L, 2 ) ) );
	else
		lua_pushnumber( L, bot->GetRangeTo( luaL_checkvector( L, 2 ) ) );

	return 1;
}

static int nextbot_GetRangeSquaredTo( lua_State *L )
{
	CBaseEntity *pEnt = luaL_checkentity( L, 1 );
	if ( !pEnt )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	CLuaNextBot *bot = dynamic_cast< CLuaNextBot * >( pEnt );
	if ( !bot )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	if ( lua_isuserdata( L, 2 ) )
		lua_pushnumber( L, bot->GetRangeSquaredTo( luaL_checkentity( L, 2 ) ) );
	else
		lua_pushnumber( L, bot->GetRangeSquaredTo( luaL_checkvector( L, 2 ) ) );

	return 1;
}

static int nextbot_StartActivity( lua_State *L )
{
	CBaseEntity *pEnt = luaL_checkentity( L, 1 );
	if ( !pEnt )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	CLuaNextBot *bot = dynamic_cast< CLuaNextBot * >( pEnt );
	if ( bot )
		bot->GetBodyInterface()->StartActivity( (Activity)luaL_checkinteger( L, 2 ) );

	return 0;
}

static int nextbot_GetActivity( lua_State *L )
{
	CBaseEntity *pEnt = luaL_checkentity( L, 1 );
	if ( !pEnt )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	CLuaNextBot *bot = dynamic_cast< CLuaNextBot * >( pEnt );
	if ( !bot )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	lua_pushinteger( L, bot ? bot->GetBodyInterface()->GetActivity() : ACT_INVALID );
	return 1;
}

static int nextbot_BecomeRagdoll( lua_State *L )
{
	CBaseEntity *pEnt = luaL_checkentity( L, 1 );
	if ( !pEnt )
	{
		lua_pushnumber( L, 0 );
		return 1;
	}

	CLuaNextBot *bot = dynamic_cast< CLuaNextBot * >( pEnt );
	if ( bot )
		bot->BecomeRagdollOnClient( vec3_origin );

	return 0;
}

static int NextBot_Index( lua_State *L )
{
	luaL_getmetatable( L, "CLuaNextBot_methods" );
	lua_pushvalue( L, 2 );
	lua_rawget( L, -2 );
	if ( !lua_isnil( L, -1 ) )
		return 1;

	lua_pop( L, 1 );

	luaL_getmetatable( L, "CBaseFlex" );
	if ( lua_istable( L, -1 ) )
	{
		lua_pushvalue( L, 2 );
		lua_rawget( L, -2 );
		return 1;
	}
	lua_pop( L, 1 );
	return 0;
}

static const luaL_Reg nextbot_methods[] = { { "GetRangeTo", nextbot_GetRangeTo }, { "GetRangeSquaredTo", nextbot_GetRangeSquaredTo }, { "StartActivity", nextbot_StartActivity }, { "GetActivity", nextbot_GetActivity },
	{ "BecomeRagdoll", nextbot_BecomeRagdoll }, { NULL, NULL } };

void RegisterScriptedNextbot( const char *className )
{
	EntityFactoryDictionary()->InstallFactory( EntityFactoryDictionary()->FindFactory( "lua_nextbot" ), className );
}

LUALIB_API int luaopen_CLuaNextBot( lua_State *L )
{
	luaL_newmetatable( L, "CLuaNextBot_methods" );
	luaL_register( L, NULL, nextbot_methods );
	lua_pop( L, 1 );

	luaL_newmetatable( L, "CLuaNextBot" );
	lua_pushcfunction( L, NextBot_Index );
	lua_setfield( L, -2, "__index" );
	lua_pop( L, 1 );

	return 1;
}
