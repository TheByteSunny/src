//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//
#ifndef LITEXTURE_H
#define LITEXTURE_H
#ifdef _WIN32
#pragma once
#endif

/* type for IMaterial functions */
typedef ITexture lua_ITexture;

/*
** access functions (stack -> C)
*/

LUA_API    lua_ITexture *lua_totexture     (lua_State *L, int idx);
LUALIB_API lua_ITexture *luaL_checktexture (lua_State *L, int narg);
LUALIB_API lua_ITexture *luaL_opttexture   (lua_State *L, int narg, lua_ITexture *def);


/*
** push functions (C -> stack)
*/

LUA_API    void          lua_pushtexture   (lua_State *L, lua_ITexture *pTexture);

#endif // LITEXTURE_H