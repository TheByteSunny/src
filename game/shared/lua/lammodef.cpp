//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "ammodef.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "lammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


/*
** access functions (stack -> C)
*/


LUA_API lua_CAmmoDef* lua_toammodef(lua_State* L, int idx)
{
	lua_CAmmoDef** ppDef = (lua_CAmmoDef**)lua_touserdata(L, idx);
	return ppDef ? *ppDef : NULL;
}

LUALIB_API lua_CAmmoDef* luaL_checkammodef(lua_State* L, int narg)
{
	lua_CAmmoDef** d = (lua_CAmmoDef**)luaL_checkudata(L, narg, "CAmmoDef");
	return *d;
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushammodef(lua_State* L, lua_CAmmoDef* pAmmoDef)
{
	if (!pAmmoDef)
	{
		lua_pushnil(L);
		return;
	}

	lua_CAmmoDef** ppDef = (lua_CAmmoDef**)lua_newuserdata(L, sizeof(lua_CAmmoDef*));
	*ppDef = pAmmoDef;
	luaL_getmetatable(L, "CAmmoDef");
	lua_setmetatable(L, -2);
}

static int AmmoDef_Index(lua_State* L)
{
	lua_pushinteger(L, luaL_checkammodef(L, 1)->Index(luaL_checkstring(L, 2)));
	return 1;
}

static int AmmoDef_PlrDamage(lua_State* L)
{
	lua_pushnumber(L, luaL_checkammodef(L, 1)->PlrDamage(luaL_checkint(L, 2)));
	return 1;
}

static int AmmoDef_NPCDamage(lua_State* L)
{
	lua_pushnumber(L, luaL_checkammodef(L, 1)->NPCDamage(luaL_checkint(L, 2)));
	return 1;
}

static int AmmoDef_MaxCarry(lua_State* L)
{
	lua_pushnumber(L, luaL_checkammodef(L, 1)->MaxCarry(luaL_checkint(L, 2)));
	return 1;
}

static int AmmoDef_AddAmmoType(lua_State* L)
{
	const char* name = luaL_checkstring(L, 2);

	int dmgType     = 0;
	int tracerType  = 0;
	float plrDmg    = 0;
	float npcDmg    = 0;
	int carry       = 0;
	float impulse   = 0;
	int flags       = 0;
	int minSplash   = 4;
	int maxSplash   = 8;

	auto resolveIntOrConst = [&](int idx, int def = 0) -> int {
		if (lua_isnoneornil(L, idx)) return def;
		if (lua_isnumber(L, idx)) return (int)lua_tointeger(L, idx);

		if (lua_isstring(L, idx)) {
			const char* s = lua_tostring(L, idx);

			lua_getglobal(L, s);
			if (lua_isnumber(L, -1)) {
				int val = (int)lua_tointeger(L, -1);
				lua_pop(L, 1);
				return val;
			}
			lua_pop(L, 1);

			// parse literal (decimal vai hex)
			char* end;
			long v = strtol(s, &end, 0);
			if (*end == '\0') return (int)v;
		}
		return def;
	};

	auto resolveFloatOrSkill = [&](int idx, float def = 0.0f) -> float {
		if (lua_isnoneornil(L, idx)) return def;
		if (lua_isnumber(L, idx)) return (float)lua_tonumber(L, idx);

		if (lua_isstring(L, idx)) {
			const char* s = lua_tostring(L, idx);

			lua_getglobal(L, s);
			if (lua_isnumber(L, -1)) {
				float val = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
				return val;
			}
			lua_pop(L, 1);

			ConVar* cvar = g_pCVar->FindVar(s);
			if (cvar)
				return cvar->GetFloat();

			char* end;
			double v = strtod(s, &end);
			if (*end == '\0') return (float)v;
		}
		return def;
	};

	dmgType    = resolveIntOrConst(3, 0);
	tracerType = resolveIntOrConst(4, 0);
	plrDmg     = resolveFloatOrSkill(5, 0);
	npcDmg     = resolveFloatOrSkill(6, 0);
	carry      = resolveIntOrConst(7, 0);
	impulse    = resolveFloatOrSkill(8, 0);
	flags      = resolveIntOrConst(9, 0);
	minSplash  = resolveIntOrConst(10, 4);
	maxSplash  = resolveIntOrConst(11, 8);

	luaL_checkammodef(L, 1)->AddAmmoType(
		name, dmgType, tracerType,
		plrDmg, npcDmg, carry,
		impulse, flags, minSplash, maxSplash
	);
	return 0;
}

static int AmmoDef___tostring(lua_State* L)
{
	lua_pushfstring(L, "CAmmoDef: %p", luaL_checkammodef(L, 1));
	return 1;
}

static const luaL_Reg AmmoDef_meta[] = {
	{"Index", AmmoDef_Index},
	{"PlrDamage", AmmoDef_PlrDamage},
	{"NPCDamage", AmmoDef_NPCDamage},
	{"MaxCarry", AmmoDef_MaxCarry},
	{"AddAmmoType", AmmoDef_AddAmmoType},
	{"__tostring", AmmoDef___tostring},
	{NULL, NULL}
};

static int luasrc_AmmoDef(lua_State* L)
{
	lua_pushammodef(L, GetAmmoDef());
	return 1;
}

static const luaL_Reg AmmoDef_funcs[] = {
	{"AmmoDef", luasrc_AmmoDef},
	{NULL, NULL}
};

LUALIB_API int luaopen_AmmoDef(lua_State* L)
{
	luaL_newmetatable(L, LUA_AMMODEFLIB_NAME);
	luaL_register(L, NULL, AmmoDef_meta);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "ammodef");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "_G", AmmoDef_funcs);
	lua_pop(L, 1);
	return 1;
}
