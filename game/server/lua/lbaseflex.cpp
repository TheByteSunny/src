//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Try to bind cbaseflex to Lua
//
//=============================================================================//

#define lbaseflex_cpp

#include "cbase.h"
#include "luamanager.h"
#include "lbaseflex.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// YourLocalSunny: Damn
LUA_API CBaseFlex *lua_toflex(lua_State *L, int idx) {
  CBaseHandle *hEntity = (CBaseHandle *)lua_touserdata(L, idx);
  if (!hEntity)
    return NULL;

  CBaseEntity *pEnt = hEntity->Get();
  if (!pEnt)
    return NULL;

  return dynamic_cast<CBaseFlex *>(pEnt);
}

LUA_API void lua_pushflex(lua_State *L, CBaseFlex *pEntity) {
  if (!pEntity) {
    lua_pushnil(L);
    return;
  }

  CBaseHandle *hEntity = (CBaseHandle *)lua_newuserdata(L, sizeof(CBaseHandle));
  hEntity->Set(pEntity);

  luaL_getmetatable(L, "CBaseFlex");
  lua_setmetatable(L, -2);
}

// YourLocalSunny: Check
LUALIB_API CBaseFlex *luaL_checkflex(lua_State *L, int narg) {
  CBaseFlex *p = lua_toflex(L, narg);
  if (!p)
    luaL_argerror(L, narg, "CBaseFlex expected, got NULL/invalid entity");
  return p;
}

static int CBaseFlex_GetFlexWeight(lua_State *L) {
  CBaseFlex *ent = luaL_checkflex(L, 1);
  int id = luaL_checkinteger(L, 2);

  lua_pushnumber(L, ent->GetFlexWeight(id));
  return 1;
}

static int CBaseFlex_SetFlexWeight(lua_State *L) {
  CBaseFlex *ent = luaL_checkflex(L, 1);
  int id = luaL_checkinteger(L, 2);
  float value = luaL_checknumber(L, 3);

  ent->SetFlexWeight(id, value);
  return 0;
}

static int CBaseFlex_GetNumFlexControllers(lua_State *L) {
  lua_pushinteger(L, luaL_checkflex(L, 1)->GetNumFlexControllers());
  return 1;
}

static int CBaseFlex___index(lua_State *L) {
  CBaseFlex *ent = lua_toflex(L, 1);
  if (!ent) {
    lua_pushstring(L, "attempt to index NULL CBaseFlex");
    return lua_error(L);
  }
  
  lua_getmetatable(L, 1);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);

  if (!lua_isnil(L, -1)) {
    lua_remove(L, -2);
    return 1;
  }
  lua_pop(L, 2);

  luaL_getmetatable(L, "CBaseAnimating");
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);

  if (!lua_isnil(L, -1)) {
    lua_remove(L, -2);
    return 1;
  }
  lua_pop(L, 2);

  luaL_getmetatable(L, "CBaseEntity");
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);

  if (!lua_isnil(L, -1)) {
    lua_remove(L, -2);
    return 1;
  }

  return 1;
}

static int CBaseFlex___newindex(lua_State *L) {
  luaL_error(L, "CBaseFlex: cannot assign new fields");
  return 0;
}

static int CBaseFlex___eq(lua_State *L) {
  lua_pushboolean(L, lua_toflex(L, 1) == lua_toflex(L, 2));
  return 1;
}

static int CBaseFlex___tostring(lua_State *L) {
  CBaseFlex *ent = lua_toflex(L, 1);

  if (!ent)
    lua_pushstring(L, "NULL");
  else
    lua_pushfstring(L, "CBaseFlex: %d \"%s\"", ent->entindex(), ent->GetClassname());

  return 1;
}

static const luaL_Reg CBaseFlex_meta[] = {
  {"GetFlexWeight", CBaseFlex_GetFlexWeight},
  {"SetFlexWeight", CBaseFlex_SetFlexWeight},
  {"GetNumFlexControllers", CBaseFlex_GetNumFlexControllers},

  {"__index", CBaseFlex___index},
  {"__newindex", CBaseFlex___newindex},
  {"__eq", CBaseFlex___eq},
  {"__tostring", CBaseFlex___tostring},

  {NULL, NULL}
};

LUALIB_API int luaopen_CBaseFlex(lua_State *L) {
  luaL_newmetatable(L, "CBaseFlex");
  luaL_register(L, NULL, CBaseFlex_meta);

  lua_pushstring(L, "entity");
  lua_setfield(L, -2, "__type");

  lua_pop(L, 1);
  return 1;
}
