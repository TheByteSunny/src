//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "lbaseflex.h"
#include "baseflex.h"
#include "lbaseanimating.h"
#include "mathlib/lvector.h"
#include "lvphysics_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/

LUA_API lua_CBaseFlex *lua_toflex( lua_State *L, int idx )
{
	CBaseHandle *hEntity = dynamic_cast< CBaseHandle * >( (CBaseHandle *)lua_touserdata( L, idx ) );
	if ( hEntity == NULL )
		return NULL;
	return dynamic_cast< lua_CBaseFlex * >( hEntity->Get() );
}

/*
** push functions (C -> stack)
*/

LUA_API void lua_pushflex( lua_State *L, CBaseFlex *pEntity )
{
	CBaseHandle *hEntity = (CBaseHandle *)lua_newuserdata( L, sizeof( CBaseHandle ) );
	hEntity->Set( pEntity );
	luaL_getmetatable( L, "CBaseFlex" );
	lua_setmetatable( L, -2 );
}

LUALIB_API lua_CBaseFlex *luaL_checkflex( lua_State *L, int narg )
{
	lua_CBaseFlex *d = lua_toflex( L, narg );
	if ( d == NULL )
		luaL_argerror( L, narg, "CBaseFlex expected, got NULL entity" );
	return d;
}

static int CBaseFlex_SetFlexWeight_ByName( lua_State *L )
{
	luaL_checkflex( L, 1 )->SetFlexWeight( const_cast< char * >( luaL_checkstring( L, 2 ) ), (float)luaL_checknumber( L, 3 ) );
	return 0;
}

static int CBaseFlex_SetFlexWeight_ByIndex( lua_State *L )
{
	luaL_checkflex( L, 1 )->SetFlexWeight( (LocalFlexController_t)luaL_checkinteger( L, 2 ), (float)luaL_checknumber( L, 3 ) );
	return 0;
}

static int CBaseFlex_GetFlexWeight_ByName( lua_State *L )
{
	lua_pushnumber( L, luaL_checkflex( L, 1 )->GetFlexWeight( const_cast< char * >( luaL_checkstring( L, 2 ) ) ) );
	return 1;
}

static int CBaseFlex_GetFlexWeight_ByIndex( lua_State *L )
{
	lua_pushnumber( L, luaL_checkflex( L, 1 )->GetFlexWeight( (LocalFlexController_t)luaL_checkinteger( L, 2 ) ) );
	return 1;
}

static int CBaseFlex_FindFlexController( lua_State *L )
{
	lua_pushinteger( L, luaL_checkflex( L, 1 )->FindFlexController( luaL_checkstring( L, 2 ) ) );
	return 1;
}

static int CBaseFlex_SetViewtarget( lua_State *L )
{
	luaL_checkflex( L, 1 )->SetViewtarget( luaL_checkvector( L, 2 ) );
	return 0;
}

static int CBaseFlex_GetViewtarget( lua_State *L )
{
	lua_pushvector( L, luaL_checkflex( L, 1 )->GetViewtarget() );
	return 1;
}

static int CBaseFlex_Blink( lua_State *L )
{
	luaL_checkflex( L, 1 )->Blink();
	return 0;
}

static int CBaseFlex_PlayScene( lua_State *L )
{
	lua_pushnumber( L, luaL_checkflex( L, 1 )->PlayScene( luaL_checkstring( L, 2 ), luaL_checknumber( L, 3 ) ) );
	return 1;
}

static int CBaseFlex_ClearSceneEvents( lua_State *L )
{
	luaL_checkflex( L, 1 )->ClearSceneEvents( NULL, lua_toboolean( L, 2 ) );
	return 0;
}

static const luaL_Reg CBaseFlexmeta[] = { { "SetFlexWeight", CBaseFlex_SetFlexWeight_ByName }, { "SetFlexWeightByIndex", CBaseFlex_SetFlexWeight_ByIndex }, { "GetFlexWeight", CBaseFlex_GetFlexWeight_ByName },
	{ "GetFlexWeightByIndex", CBaseFlex_GetFlexWeight_ByIndex }, { "FindFlexController", CBaseFlex_FindFlexController }, { "SetViewtarget", CBaseFlex_SetViewtarget }, { "GetViewtarget", CBaseFlex_GetViewtarget },
	{ "Blink", CBaseFlex_Blink }, { "PlayScene", CBaseFlex_PlayScene }, { "ClearSceneEvents", CBaseFlex_ClearSceneEvents }, { "__index", NULL }, { "__newindex", NULL }, { NULL, NULL } };

LUALIB_API int luaopen_CBaseFlex( lua_State *L )
{
	luaL_newmetatable( L, "CBaseFlex" );
	luaL_register( L, NULL, CBaseFlexmeta );
	lua_pushstring( L, "entity" );
	lua_setfield( L, -2, "__type" ); /* metatable.__type = "entity" */

	luaL_getmetatable( L, "CBaseAnimatingOverlay" );
	if ( lua_istable( L, -1 ) )
	{
		lua_setfield( L, -2, "__index" );
	}

	lua_pop( L, 1 );
	return 1;
}