//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "lbaseentity_shared.h"
#include "mathlib/lvector.h"
#include "lnextbotpath.h"
#include "lnextbot.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CLuaPathCost::CLuaPathCost( lua_State *L, INextBot *bot, int funcRef ) : m_L( L ), m_bot( bot ), m_luaFuncRef( funcRef )
{
	m_L = L;
	m_bot = bot;
	m_luaFuncRef = funcRef;
}

float CLuaPathCost::operator()( CNavArea *area, CNavArea *fromArea, const CNavLadder *ladder, const CFuncElevator *elevator, float length ) const
{
	if ( !area )
		return -1.0f;

	if ( m_luaFuncRef != LUA_NOREF )
	{
		lua_rawgeti( m_L, LUA_REGISTRYINDEX, m_luaFuncRef );
		if ( lua_isfunction( m_L, -1 ) )
		{
			lua_pushnil( m_L );
			lua_pushnil( m_L );
			if ( lua_pcall( m_L, 2, 1, 0 ) == 0 )
			{
				float result = lua_tonumber( m_L, -1 );
				lua_pop( m_L, 1 );
				return result;
			}
			lua_pop( m_L, 1 );
		}
		else
		{
			lua_pop( m_L, 1 );
		}
	}

	if ( fromArea == NULL )
		return 0.0f;

	float dist;
	if ( ladder )
		dist = ladder->m_length;
	else if ( length > 0.0f )
		dist = length;
	else
		dist = ( area->GetCenter() - fromArea->GetCenter() ).Length();

	float cost = dist + fromArea->GetCostSoFar();

	if ( area->GetAttributes() & NAV_MESH_AVOID )
		cost += 1000.0f;

	return cost;
}


/*
** push functions (C -> stack)
*/

void lua_pushnextbotpath( lua_State *L, PathFollower *pPath )
{
	if ( !pPath )
	{
		lua_pushnil( L );
		return;
	}
	PathFollower **pp = (PathFollower **)lua_newuserdata( L, sizeof( PathFollower * ) );
	*pp = pPath;
	luaL_getmetatable( L, LUA_NEXTBOT_PATHLIBNAME );
	lua_setmetatable( L, -2 );
}

/*
** access functions (stack -> C)
*/

PathFollower *luaL_checknextbotpath( lua_State *L, int narg )
{
	PathFollower **pp = (PathFollower **)luaL_checkudata( L, narg, LUA_NEXTBOT_PATHLIBNAME );
	if ( !pp || !*pp )
		luaL_error( L, "invalid PathFollower" );
	return *pp;
}

static int lua_Path_new( lua_State *L )
{
	const char	 *pszType = luaL_checkstring( L, 1 );
	PathFollower *pPath = NULL;

	if ( Q_strcmp( pszType, "Follow" ) == 0 )
		pPath = new PathFollower();
	else if ( Q_strcmp( pszType, "Chase" ) == 0 )
		pPath = new ChasePath();
	else
		luaL_error( L, "Path: invalid type '%s' (use 'Follow' o 'Chase')", pszType );

	lua_pushnextbotpath( L, pPath );
	return 1;
}

static int path_Compute( lua_State *L )
{
	PathFollower *path = luaL_checknextbotpath( L, 1 );
	INextBot	 *bot = luaL_checknextbot( L, 2 );
	Vector		  goalPos = luaL_checkvector( L, 3 );

	// if bot has no known area, dont compute
	if ( !bot->GetEntity()->GetLastKnownArea() )
		return 0;

	int costRef = LUA_NOREF;
	if ( lua_isfunction( L, 4 ) )
	{
		lua_pushvalue( L, 4 );
		costRef = luaL_ref( L, LUA_REGISTRYINDEX );
	}

	CLuaPathCost cost( L, bot, costRef );
	path->Compute( bot, goalPos, cost );

	if ( costRef != LUA_NOREF )
		luaL_unref( L, LUA_REGISTRYINDEX, costRef );

	return 0;
}

static int path_Update( lua_State *L )
{
	luaL_checknextbotpath( L, 1 )->Update( luaL_checknextbot( L, 2 ) );
	return 0;
}

static int path_IsValid( lua_State *L )
{
	lua_pushboolean( L, luaL_checknextbotpath( L, 1 )->IsValid() );
	return 1;
}

static int path_Invalidate( lua_State *L )
{
	luaL_checknextbotpath( L, 1 )->Invalidate();
	return 0;
}

static int path_GetLength( lua_State *L )
{
	lua_pushnumber( L, luaL_checknextbotpath( L, 1 )->GetLength() );
	return 1;
}

static int path_GetAge( lua_State *L )
{
	lua_pushnumber( L, luaL_checknextbotpath( L, 1 )->GetAge() );
	return 1;
}

static int path_GetStart( lua_State *L )
{
	PathFollower		*path = luaL_checknextbotpath( L, 1 );
	const Path::Segment *seg = path->FirstSegment();
	if ( seg )
		lua_pushvector( L, seg->pos );
	else
		lua_pushnil( L );
	return 1;
}

static int path_GetEnd( lua_State *L )
{
	PathFollower		*path = luaL_checknextbotpath( L, 1 );
	const Path::Segment *seg = path->LastSegment();
	if ( seg )
		lua_pushvector( L, seg->pos );
	else
		lua_pushnil( L );
	return 1;
}

static int path_SetMinLookAheadDistance( lua_State *L )
{
	luaL_checknextbotpath( L, 1 )->SetMinLookAheadDistance( luaL_checknumber( L, 2 ) );
	return 0;
}

static int path_SetGoalTolerance( lua_State *L )
{
	luaL_checknextbotpath( L, 1 )->SetGoalTolerance( luaL_checknumber( L, 2 ) );
	return 0;
}

static int path_GetClosestPosition( lua_State *L )
{
	PathFollower *path = luaL_checknextbotpath( L, 1 );
	Vector		  pos = luaL_checkvector( L, 2 );
	lua_pushvector( L, path->GetClosestPosition( pos ) );
	return 1;
}

static int path_GetCursorPosition( lua_State *L )
{
	PathFollower *path = luaL_checknextbotpath( L, 1 );
	lua_pushvector( L, path->GetPosition( path->GetCursorPosition() ) );
	return 1;
}

static int path_Draw( lua_State *L )
{
	luaL_checknextbotpath( L, 1 )->Draw();
	return 0;
}

static int path_Chase( lua_State *L )
{
	PathFollower *path = luaL_checknextbotpath( L, 1 );
	INextBot	 *bot = luaL_checknextbot( L, 2 );
	CBaseEntity	 *target = luaL_checkentity( L, 3 );

	ChasePath *chase = dynamic_cast< ChasePath * >( path );
	if ( chase )
	{
		CLuaPathCost cost( L, bot );
		chase->Update( bot, target, cost );
	}
	return 0;
}

static int path_gc( lua_State *L )
{
	PathFollower **pp = (PathFollower **)lua_touserdata( L, 1 );
	if ( pp && *pp )
	{
		delete *pp;
		*pp = NULL;
	}
	return 0;
}

static int path_tostring( lua_State *L )
{
	lua_pushstring( L, LUA_NEXTBOT_PATHLIBNAME );
	return 1;
}

static const luaL_Reg path_methods[] = { { "Compute", path_Compute }, { "Update", path_Update }, { "IsValid", path_IsValid }, { "Invalidate", path_Invalidate }, { "GetLength", path_GetLength }, { "GetAge", path_GetAge },
	{ "GetStart", path_GetStart }, { "GetEnd", path_GetEnd }, { "SetMinLookAheadDistance", path_SetMinLookAheadDistance }, { "SetGoalTolerance", path_SetGoalTolerance }, { "GetClosestPosition", path_GetClosestPosition },
	{ "GetCursorPosition", path_GetCursorPosition }, { "Draw", path_Draw }, { "Chase", path_Chase }, { "__gc", path_gc }, { "__tostring", path_tostring }, { NULL, NULL } };


LUALIB_API int luaopen_CNextBotPath( lua_State *L )
{
	luaL_newmetatable( L, LUA_NEXTBOT_PATHLIBNAME );
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );
	luaL_register( L, NULL, path_methods );
	lua_pushstring( L, LUA_NEXTBOT_PATHLIBNAME );
	lua_setfield( L, -2, "__type" );
	lua_pop( L, 1 );

	lua_register( L, "Path", lua_Path_new );

	return 1;
}
