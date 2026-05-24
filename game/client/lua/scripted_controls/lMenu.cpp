//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lMenu.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LMenu::LMenu(Panel* parent, const char* panelName, lua_State* L) : Menu(parent, panelName)
{
#if defined(LUA_SDK)
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif
}

LMenu::~LMenu()
{
#if defined(LUA_SDK)
	lua_unref(m_lua_State, m_nTableReference);
#endif
}


/*
** access functions (stack -> C)
*/

LUA_API lua_Menu* lua_tomenu(lua_State* L, int idx)
{
	PHandle* phPanel = dynamic_cast<PHandle*>((PHandle*)lua_touserdata(L, idx));
	if (phPanel == NULL) return NULL;
	return dynamic_cast<lua_Menu*>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushmenu(lua_State* L, Menu* pMenu)
{
	LMenu* plMenu = dynamic_cast<LMenu*>(pMenu);
	if (plMenu) ++plMenu->m_nRefCount;
	PHandle* phPanel = (PHandle*)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pMenu);
	luaL_getmetatable(L, "Menu");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_Menu* luaL_checkmenu(lua_State* L, int narg)
{
	lua_Menu* d = lua_tomenu(L, narg);
	if (d == NULL) luaL_argerror(L, narg, "Menu expected, got INVALID_PANEL");
	return d;
}

static int Menu_AddMenuItem(lua_State* L)
{
	Menu* menu = luaL_checkmenu(L, 1);
	const char* text = luaL_checkstring(L, 2);

	int id = menu->AddMenuItem(text, menu, NULL);

	lua_pushinteger(L, id);
	return 1;
}

static int Menu_AddSeparator(lua_State* L)
{
	luaL_checkmenu(L, 1)->AddSeparator();
	return 0;
}

static int Menu_SetVisible(lua_State* L)
{
	luaL_checkmenu(L, 1)->SetVisible(luaL_checkboolean(L, 2));
	return 0;
}

static int Menu___index(lua_State* L)
{
	Menu* pMenu = lua_tomenu(L, 1);
	if (pMenu == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LMenu* plMenu = dynamic_cast<LMenu*>(pMenu);
	if (plMenu && plMenu->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plMenu->m_nTableReference);
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			lua_getmetatable(L, 1);
			lua_pushvalue(L, 2);
			lua_gettable(L, -2);
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 2);
				luaL_getmetatable(L, "Panel");
				lua_pushvalue(L, 2);
				lua_gettable(L, -2);
			}
		}
	}
	else
	{
		lua_getmetatable(L, 1);
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			luaL_getmetatable(L, "Panel");
			lua_pushvalue(L, 2);
			lua_gettable(L, -2);
		}
	}
	return 1;
}

static int Menu___newindex(lua_State* L)
{
	Menu* pMenu = lua_tomenu(L, 1);
	if (pMenu == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LMenu* plMenu = dynamic_cast<LMenu*>(pMenu);
	if (plMenu)
	{
		if (plMenu->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plMenu->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plMenu->m_nTableReference);
		lua_pushvalue(L, 3);
		lua_setfield(L, -2, luaL_checkstring(L, 2));
		lua_pop(L, 1);
		return 0;
	}
	else
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index non-scripted panel", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
}

static int Menu___gc(lua_State* L)
{
	LMenu* plMenu = dynamic_cast<LMenu*>(lua_tomenu(L, 1));
	if (plMenu)
	{
		--plMenu->m_nRefCount;
		if (plMenu->m_nRefCount <= 0) delete plMenu;
	}
	return 0;
}

static int Menu___eq(lua_State* L)
{
	lua_pushboolean(L, lua_tomenu(L, 1) == lua_tomenu(L, 2));
	return 1;
}

static int Menu___tostring(lua_State* L)
{
	Menu* pMenu = lua_tomenu(L, 1);
	if (pMenu == NULL) lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char* pName = pMenu->GetName();
		if (Q_strcmp(pName, "") == 0) pName = "(no name)";
		lua_pushfstring(L, "Menu: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg MenuMeta[] =
{
	{"AddMenuItem", Menu_AddMenuItem},
	{"AddSeparator", Menu_AddSeparator},
	{"SetVisible", Menu_SetVisible},
	{"__index", Menu___index},
	{"__newindex", Menu___newindex},
	{"__gc", Menu___gc},
	{"__eq", Menu___eq},
	{"__tostring", Menu___tostring},
	{NULL, NULL}
};

static int luasrc_Menu(lua_State* L)
{
	Menu* pMenu = lua_tomenu(L, 1);
	if (!pMenu)
	{
		pMenu = new LMenu(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
						   luaL_checkstring(L, 2), L);
	}
	lua_pushmenu(L, pMenu);
	return 1;
}

static const luaL_Reg Menu_funcs[] =
{
	{"Menu", luasrc_Menu},
	{NULL, NULL}
};

/* open Menu object */
LUALIB_API int luaopen_vgui_Menu(lua_State* L)
{
	luaL_newmetatable(L, "Menu");
	luaL_register(L, NULL, MenuMeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", Menu_funcs);
	lua_pop(L, 2);
	return 1;
}
