//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#define lmovedata_cpp
#include "cbase.h"
#include "igamemovement.h"
#include "lua.hpp"
#include "lmovedata.h"
#include "mathlib/lvector.h"
#include "lbaseentity_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/
LUA_API CMoveData *lua_tomovedata(lua_State *L, int idx)
{
	CMoveData **ppMv = (CMoveData **)lua_touserdata(L, idx);
	return ppMv ? *ppMv : NULL;
}

/*
** push functions (C -> stack)
*/
LUA_API void lua_pushmovedata(lua_State *L, CMoveData *mv)
{
	if (mv == NULL)
	{
		lua_pushnil(L);
		return;
	}
	CMoveData **ppMv = (CMoveData **)lua_newuserdata(L, sizeof(CMoveData *));
	*ppMv = mv;
	luaL_getmetatable(L, LUA_MOVEDATALIBNAME);
	lua_setmetatable(L, -2);
}

LUALIB_API CMoveData *luaL_checkmovedata(lua_State *L, int narg)
{
	CMoveData **ppMv = (CMoveData **)luaL_checkudata(L, narg, LUA_MOVEDATALIBNAME);
	return *ppMv;
}

/*
** CMoveData methods
*/

static int CMoveData_GetButtons(lua_State *L)
{
	lua_pushinteger(L, luaL_checkmovedata(L, 1)->m_nButtons);
	return 1;
}

static int CMoveData_SetButtons(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_nButtons = (int)luaL_checkinteger(L, 2);
	return 0;
}

static int CMoveData_GetOldButtons(lua_State *L)
{
	lua_pushinteger(L, luaL_checkmovedata(L, 1)->m_nOldButtons);
	return 1;
}

static int CMoveData_SetOldButtons(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_nOldButtons = (int)luaL_checkinteger(L, 2);
	return 0;
}

static int CMoveData_GetViewAngles(lua_State *L)
{
	lua_pushangle(L, luaL_checkmovedata(L, 1)->m_vecViewAngles);
	return 1;
}

static int CMoveData_SetViewAngles(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_vecViewAngles = luaL_checkangle(L, 2);
	return 0;
}

static int CMoveData_GetAbsViewAngles(lua_State *L)
{
	lua_pushangle(L, luaL_checkmovedata(L, 1)->m_vecAbsViewAngles);
	return 1;
}

static int CMoveData_GetForwardMove(lua_State *L)
{
	lua_pushnumber(L, luaL_checkmovedata(L, 1)->m_flForwardMove);
	return 1;
}

static int CMoveData_SetForwardMove(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_flForwardMove = (float)luaL_checknumber(L, 2);
	return 0;
}

static int CMoveData_GetSideMove(lua_State *L)
{
	lua_pushnumber(L, luaL_checkmovedata(L, 1)->m_flSideMove);
	return 1;
}

static int CMoveData_SetSideMove(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_flSideMove = (float)luaL_checknumber(L, 2);
	return 0;
}

static int CMoveData_GetUpMove(lua_State *L)
{
	lua_pushnumber(L, luaL_checkmovedata(L, 1)->m_flUpMove);
	return 1;
}

static int CMoveData_SetUpMove(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_flUpMove = (float)luaL_checknumber(L, 2);
	return 0;
}

static int CMoveData_GetMaxSpeed(lua_State *L)
{
	lua_pushnumber(L, luaL_checkmovedata(L, 1)->m_flMaxSpeed);
	return 1;
}

static int CMoveData_SetMaxSpeed(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_flMaxSpeed = (float)luaL_checknumber(L, 2);
	return 0;
}

static int CMoveData_GetClientMaxSpeed(lua_State *L)
{
	lua_pushnumber(L, luaL_checkmovedata(L, 1)->m_flClientMaxSpeed);
	return 1;
}

static int CMoveData_SetClientMaxSpeed(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_flClientMaxSpeed = (float)luaL_checknumber(L, 2);
	return 0;
}

static int CMoveData_GetVelocity(lua_State *L)
{
	lua_pushvector(L, luaL_checkmovedata(L, 1)->m_vecVelocity);
	return 1;
}

static int CMoveData_SetVelocity(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_vecVelocity = luaL_checkvector(L, 2);
	return 0;
}

static int CMoveData_GetOrigin(lua_State *L)
{
	lua_pushvector(L, luaL_checkmovedata(L, 1)->GetAbsOrigin());
	return 1;
}

static int CMoveData_SetOrigin(lua_State *L)
{
	luaL_checkmovedata(L, 1)->SetAbsOrigin(luaL_checkvector(L, 2));
	return 0;
}

static int CMoveData_GetFlags(lua_State *L)
{
    // todo: fix
	lua_pushinteger(L, luaL_checkmovedata(L, 1)->m_nPlayerHandle.GetEntryIndex());
	return 1;
}

static int CMoveData_GetConstraintRadius(lua_State *L)
{
	lua_pushnumber(L, luaL_checkmovedata(L, 1)->m_flConstraintRadius);
	return 1;
}

static int CMoveData_SetConstraintRadius(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_flConstraintRadius = (float)luaL_checknumber(L, 2);
	return 0;
}

static int CMoveData_GetConstraintSpeedFactor(lua_State *L)
{
	lua_pushnumber(L, luaL_checkmovedata(L, 1)->m_flConstraintSpeedFactor);
	return 1;
}

static int CMoveData_SetConstraintSpeedFactor(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_flConstraintSpeedFactor = (float)luaL_checknumber(L, 2);
	return 0;
}

static int CMoveData_GetConstraintCenter(lua_State *L)
{
	lua_pushvector(L, luaL_checkmovedata(L, 1)->m_vecConstraintCenter);
	return 1;
}

static int CMoveData_SetConstraintCenter(lua_State *L)
{
	luaL_checkmovedata(L, 1)->m_vecConstraintCenter = luaL_checkvector(L, 2);
	return 0;
}

static int CMoveData_KeyDown(lua_State *L)
{
	CMoveData *mv = luaL_checkmovedata(L, 1);
	int btn = (int)luaL_checkinteger(L, 2);
	lua_pushboolean(L, (mv->m_nButtons & btn) != 0);
	return 1;
}

static int CMoveData_KeyPressed(lua_State *L)
{
	CMoveData *mv = luaL_checkmovedata(L, 1);
	int btn = (int)luaL_checkinteger(L, 2);
	lua_pushboolean(L, ((mv->m_nButtons & btn) != 0) && ((mv->m_nOldButtons & btn) == 0));
	return 1;
}

static int CMoveData_KeyReleased(lua_State *L)
{
	CMoveData *mv = luaL_checkmovedata(L, 1);
	int btn = (int)luaL_checkinteger(L, 2);
	lua_pushboolean(L, ((mv->m_nButtons & btn) == 0) && ((mv->m_nOldButtons & btn) != 0));
	return 1;
}

static int CMoveData___tostring(lua_State *L)
{
	CMoveData *mv = luaL_checkmovedata(L, 1);
	lua_pushfstring(L, "CMoveData: %p", mv);
	return 1;
}

static const luaL_Reg CMoveData_meta[] = {
	{"GetButtons",              CMoveData_GetButtons},
	{"SetButtons",              CMoveData_SetButtons},
	{"GetOldButtons",           CMoveData_GetOldButtons},
	{"SetOldButtons",           CMoveData_SetOldButtons},
	{"KeyDown",                 CMoveData_KeyDown},
	{"KeyPressed",              CMoveData_KeyPressed},
	{"KeyReleased",             CMoveData_KeyReleased},

    {"GetViewAngles",           CMoveData_GetViewAngles},
	{"SetViewAngles",           CMoveData_SetViewAngles},
	{"GetAbsViewAngles",        CMoveData_GetAbsViewAngles},

    {"GetForwardMove",          CMoveData_GetForwardMove},
	{"SetForwardMove",          CMoveData_SetForwardMove},
	{"GetSideMove",             CMoveData_GetSideMove},
	{"SetSideMove",             CMoveData_SetSideMove},
	{"GetUpMove",               CMoveData_GetUpMove},
	{"SetUpMove",               CMoveData_SetUpMove},
	{"GetMaxSpeed",             CMoveData_GetMaxSpeed},
	{"SetMaxSpeed",             CMoveData_SetMaxSpeed},
	{"GetClientMaxSpeed",       CMoveData_GetClientMaxSpeed},
	{"SetClientMaxSpeed",       CMoveData_SetClientMaxSpeed},

    {"GetVelocity",             CMoveData_GetVelocity},
	{"SetVelocity",             CMoveData_SetVelocity},
	{"GetOrigin",               CMoveData_GetOrigin},
	{"SetOrigin",               CMoveData_SetOrigin},

    {"GetConstraintRadius",     CMoveData_GetConstraintRadius},
	{"SetConstraintRadius",     CMoveData_SetConstraintRadius},
	{"GetConstraintSpeedFactor",CMoveData_GetConstraintSpeedFactor},
	{"SetConstraintSpeedFactor",CMoveData_SetConstraintSpeedFactor},
	{"GetConstraintCenter",     CMoveData_GetConstraintCenter},
	{"SetConstraintCenter",     CMoveData_SetConstraintCenter},

    {"__tostring",              CMoveData___tostring},
	{NULL, NULL}
};

LUALIB_API int luaopen_CMoveData(lua_State *L)
{
	luaL_newmetatable(L, LUA_MOVEDATALIBNAME);
	luaL_register(L, NULL, CMoveData_meta);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "movedata");
	lua_setfield(L, -2, "__type");
	lua_pop(L, 1);
	return 1;
}
