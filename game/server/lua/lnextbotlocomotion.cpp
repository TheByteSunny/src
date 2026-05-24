//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luasrclib.h"
#include "lnextbotlocomotion.h"
#include "mathlib/lvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CLuaNextBotLocomotion::CLuaNextBotLocomotion( INextBot *bot ) : NextBotGroundLocomotion( bot )
{
	m_flStepHeight = 18.0f;
	m_flJumpHeight = 180.0f;
	m_flDeathDrop = 200.0f;
	m_flMaxAccel = 500.0f;
	m_flMaxDecel = 500.0f;
}

void CLuaNextBotLocomotion::SetStepHeight( float f )
{
	m_flStepHeight = f;
}

void CLuaNextBotLocomotion::SetJumpHeight( float f )
{
	m_flJumpHeight = f;
}

void CLuaNextBotLocomotion::SetDeathDropHeight( float f )
{
	m_flDeathDrop = f;
}

void CLuaNextBotLocomotion::SetDeceleration( float f )
{
	m_flMaxDecel = f;
}

void CLuaNextBotLocomotion::SetMaxAcceleration( float f )
{
	m_flMaxAccel = f;
}


/*
** push functions (C -> stack)
*/

void lua_pushlocomotion( lua_State *L, CLuaNextBotLocomotion *pLoco )
{
	if ( !pLoco )
	{
		lua_pushnil( L );
		return;
	}

	CLuaNextBotLocomotion **pp = (CLuaNextBotLocomotion **)lua_newuserdata( L, sizeof( CLuaNextBotLocomotion * ) );
	*pp = pLoco;
	luaL_getmetatable( L, LUA_NEXTBOT_LOCOMOTIONLIBNAME );
	lua_setmetatable( L, -2 );
}

/*
** access functions (stack -> C)
*/

CLuaNextBotLocomotion *luaL_checklocomotion( lua_State *L, int narg )
{
	CLuaNextBotLocomotion **pp = (CLuaNextBotLocomotion **)luaL_checkudata( L, narg, LUA_NEXTBOT_LOCOMOTIONLIBNAME );
	if ( !pp || !*pp )
		luaL_error( L, "invalid NextBotGroundLocomotion" );

	return *pp;
}

static int loco_SetDesiredSpeed( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->SetDesiredSpeed( luaL_checknumber( L, 2 ) );
	return 0;
}

static int loco_GetDesiredSpeed( lua_State *L )
{
	lua_pushnumber( L, luaL_checklocomotion( L, 1 )->GetDesiredSpeed() );
	return 1;
}

static int loco_Approach( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->Approach( luaL_checkvector( L, 2 ), luaL_optnumber( L, 3, 1.0f ) );
	return 0;
}

static int loco_FaceTowards( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->FaceTowards( luaL_checkvector( L, 2 ) );
	return 0;
}

static int loco_IsClimbingOrJumping( lua_State *L )
{
	lua_pushboolean( L, luaL_checklocomotion( L, 1 )->IsClimbingOrJumping() );
	return 1;
}

static int loco_Jump( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->Jump();
	return 0;
}

static int loco_GetVelocity( lua_State *L )
{
	lua_pushvector( L, luaL_checklocomotion( L, 1 )->GetVelocity() );
	return 1;
}

static int loco_GetCurrentAcceleration( lua_State *L )
{
	lua_pushvector( L, luaL_checklocomotion( L, 1 )->GetAcceleration() );
	return 1;
}

static int loco_IsAttemptingToMove( lua_State *L )
{
	lua_pushboolean( L, luaL_checklocomotion( L, 1 )->IsAttemptingToMove() );
	return 1;
}

static int loco_IsStuck( lua_State *L )
{
	lua_pushboolean( L, luaL_checklocomotion( L, 1 )->IsStuck() );
	return 1;
}

static int loco_SetAcceleration( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->SetMaxAcceleration( luaL_checknumber( L, 2 ) );
	return 0;
}

static int loco_GetAcceleration( lua_State *L )
{
	lua_pushnumber( L, luaL_checklocomotion( L, 1 )->GetMaxAcceleration() );
	return 1;
}

static int loco_SetDeceleration( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->SetDeceleration( luaL_checknumber( L, 2 ) );
	return 0;
}

static int loco_GetDeceleration( lua_State *L )
{
	lua_pushnumber( L, luaL_checklocomotion( L, 1 )->GetMaxDeceleration() );
	return 1;
}

static int loco_SetStepHeight( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->SetStepHeight( luaL_checknumber( L, 2 ) );
	return 0;
}

static int loco_GetStepHeight( lua_State *L )
{
	lua_pushnumber( L, luaL_checklocomotion( L, 1 )->GetStepHeight() );
	return 1;
}

static int loco_SetJumpHeight( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->SetJumpHeight( luaL_checknumber( L, 2 ) );
	return 0;
}

static int loco_GetJumpHeight( lua_State *L )
{
	lua_pushnumber( L, luaL_checklocomotion( L, 1 )->GetMaxJumpHeight() );
	return 1;
}

static int loco_SetDeathDropHeight( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->SetDeathDropHeight( luaL_checknumber( L, 2 ) );
	return 0;
}

static int loco_GetDeathDropHeight( lua_State *L )
{
	lua_pushnumber( L, luaL_checklocomotion( L, 1 )->GetDeathDropHeight() );
	return 1;
}

static int loco_ClearStuck( lua_State *L )
{
	luaL_checklocomotion( L, 1 )->ClearStuckStatus( "Lua ClearStuck" );
	return 0;
}

static int loco_GetGroundMotionVector( lua_State *L )
{
	lua_pushvector( L, luaL_checklocomotion( L, 1 )->GetGroundMotionVector() );
	return 1;
}

static int loco_tostring( lua_State *L )
{
	lua_pushstring( L, LUA_NEXTBOT_LOCOMOTIONLIBNAME );
	return 1;
}


static const luaL_Reg loco_methods[] = { { "SetDesiredSpeed", loco_SetDesiredSpeed }, { "Approach", loco_Approach }, { "FaceTowards", loco_FaceTowards }, { "IsClimbingOrJumping", loco_IsClimbingOrJumping }, { "Jump", loco_Jump },
	{ "GetVelocity", loco_GetVelocity }, { "GetCurrentAcceleration", loco_GetCurrentAcceleration }, { "IsAttemptingToMove", loco_IsAttemptingToMove }, { "IsStuck", loco_IsStuck }, { "SetAcceleration", loco_SetAcceleration },
	{ "GetAcceleration", loco_GetAcceleration }, { "SetDeceleration", loco_SetDeceleration }, { "GetDeceleration", loco_GetDeceleration }, { "SetStepHeight", loco_SetStepHeight }, { "GetStepHeight", loco_GetStepHeight },
	{ "SetJumpHeight", loco_SetJumpHeight }, { "GetJumpHeight", loco_GetJumpHeight }, { "SetDeathDropHeight", loco_SetDeathDropHeight }, { "GetDeathDropHeight", loco_GetDeathDropHeight }, { "ClearStuck", loco_ClearStuck },
	{ "GetGroundMotionVector", loco_GetGroundMotionVector }, { "__tostring", loco_tostring }, { NULL, NULL } };


LUALIB_API int luaopen_NextBotLocomotion( lua_State *L )
{
	luaL_newmetatable( L, LUA_NEXTBOT_LOCOMOTIONLIBNAME );
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );
	luaL_register( L, NULL, loco_methods );
	lua_pushstring( L, LUA_NEXTBOT_LOCOMOTIONLIBNAME );
	lua_setfield( L, -2, "__type" );
	lua_pop( L, 1 );
	return 1;
}
