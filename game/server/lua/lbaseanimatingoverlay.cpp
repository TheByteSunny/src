//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "lbaseanimatingoverlay.h"
#include "BaseAnimatingOverlay.h"
#include "lbaseanimating.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LUA_API lua_CBaseAnimatingOverlay *lua_toanimatingoverlay( lua_State *L, int idx )
{
	CBaseHandle *hEntity = dynamic_cast< CBaseHandle * >( (CBaseHandle *)lua_touserdata( L, idx ) );
	if ( hEntity == NULL )
		return NULL;
	return dynamic_cast< lua_CBaseAnimatingOverlay * >( hEntity->Get() );
}

LUA_API void lua_pushanimatingoverlay( lua_State *L, CBaseAnimatingOverlay *pEntity )
{
	CBaseHandle *hEntity = (CBaseHandle *)lua_newuserdata( L, sizeof( CBaseHandle ) );
	hEntity->Set( pEntity );
	luaL_getmetatable( L, "CBaseAnimatingOverlay" );
	lua_setmetatable( L, -2 );
}

LUALIB_API lua_CBaseAnimatingOverlay *luaL_checkanimatingoverlay( lua_State *L, int narg )
{
	lua_CBaseAnimatingOverlay *d = lua_toanimatingoverlay( L, narg );
	if ( d == NULL )
		luaL_argerror( L, narg, "CBaseAnimatingOverlay expected, got NULL entity" );
	return d;
}

static int CBaseAnimatingOverlay_AddGesture( lua_State *L )
{
	CBaseAnimatingOverlay *pOverlay = luaL_checkanimatingoverlay( L, 1 );
	Activity			   activity = (Activity)luaL_checkinteger( L, 2 );

	int top = lua_gettop( L );

	if ( top >= 4 )
		lua_pushinteger( L, pOverlay->AddGesture( activity, (float)luaL_checknumber( L, 3 ), (bool)lua_toboolean( L, 4 ) ) );
	else if ( top >= 3 && lua_isnumber( L, 3 ) )
		lua_pushinteger( L, pOverlay->AddGesture( activity, (float)luaL_checknumber( L, 3 ) ) );
	else
		lua_pushinteger( L, pOverlay->AddGesture( activity, (bool)lua_toboolean( L, 3 ) ) );

	return 1;
}

static int CBaseAnimatingOverlay_IsPlayingGesture( lua_State *L )
{
	lua_pushboolean( L, luaL_checkanimatingoverlay( L, 1 )->IsPlayingGesture( (Activity)luaL_checkinteger( L, 2 ) ) );
	return 1;
}

static int CBaseAnimatingOverlay_RestartGesture( lua_State *L )
{
	CBaseAnimatingOverlay *pOverlay = luaL_checkanimatingoverlay( L, 1 );
	Activity			   activity = (Activity)luaL_checkinteger( L, 2 );
	int					   top = lua_gettop( L );

	if ( top >= 4 )
		pOverlay->RestartGesture( activity, lua_toboolean( L, 3 ), lua_toboolean( L, 4 ) );
	else if ( top >= 3 )
		pOverlay->RestartGesture( activity, lua_toboolean( L, 3 ) );
	else
		pOverlay->RestartGesture( activity );

	return 0;
}

static int CBaseAnimatingOverlay_RemoveAllGestures( lua_State *L )
{
	luaL_checkanimatingoverlay( L, 1 )->RemoveAllGestures();
	return 0;
}

static int CBaseAnimatingOverlay_HasActiveLayer( lua_State *L )
{
	lua_pushboolean( L, luaL_checkanimatingoverlay( L, 1 )->HasActiveLayer() );
	return 1;
}

static const luaL_Reg CBaseAnimatingOverlaymeta[] = { { "AddGesture", CBaseAnimatingOverlay_AddGesture }, { "IsPlayingGesture", CBaseAnimatingOverlay_IsPlayingGesture }, { "RestartGesture", CBaseAnimatingOverlay_RestartGesture },
	{ "RemoveAllGestures", CBaseAnimatingOverlay_RemoveAllGestures }, { "HasActiveLayer", CBaseAnimatingOverlay_HasActiveLayer }, { "__index", NULL }, { "__newindex", NULL }, { NULL, NULL } };

LUALIB_API int luaopen_CBaseAnimatingOverlay( lua_State *L )
{
	luaL_newmetatable( L, "CBaseAnimatingOverlay" );
	luaL_register( L, NULL, CBaseAnimatingOverlaymeta );
	lua_pushstring( L, "entity" );
	lua_setfield( L, -2, "__type" );

	luaL_getmetatable( L, "CBaseAnimating" );
	if ( lua_istable( L, -1 ) )
	{
		lua_setfield( L, -2, "__index" );
	}

	lua_pop( L, 1 );
	return 1;
}