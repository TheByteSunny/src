//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lImagePanel.h"
#include "lColor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LImagePanel::LImagePanel(Panel *parent, const char *panelName, lua_State *L) : ImagePanel(parent, panelName)
{
#if defined( LUA_SDK )
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif // LUA_SDK
}

LImagePanel::~LImagePanel()
{
#if defined( LUA_SDK )
	lua_unref( m_lua_State, m_nTableReference );
#endif // LUA_SDK
}


/*
** access functions (stack -> C)
*/

LUA_API lua_ImagePanel *lua_toimagepanel(lua_State *L, int idx)
{
	PHandle *phPanel = dynamic_cast<PHandle *>((PHandle *)lua_touserdata(L, idx));
	if (phPanel == NULL)
		return NULL;
	return dynamic_cast<lua_ImagePanel *>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushimagepanel(lua_State *L, ImagePanel *pImagePanel)
{
	LImagePanel *plImagePanel = dynamic_cast<LImagePanel *>(pImagePanel);
	if (plImagePanel)
		++plImagePanel->m_nRefCount;
	PHandle *phPanel = (PHandle *)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pImagePanel);
	luaL_getmetatable(L, "ImagePanel");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_ImagePanel *luaL_checkimagepanel(lua_State *L, int narg)
{
	lua_ImagePanel *d = lua_toimagepanel(L, narg);
	if (d == NULL)
		luaL_argerror(L, narg, "ImagePanel expected, got INVALID_PANEL");
	return d;
}

static int ImagePanel_SetImage(lua_State *L)
{
	luaL_checkimagepanel(L, 1)->SetImage(luaL_checkstring(L, 2));
	return 0;
}

static int ImagePanel_SetShouldScaleImage(lua_State *L)
{
	luaL_checkimagepanel(L, 1)->SetShouldScaleImage(luaL_checkboolean(L, 2));
	return 0;
}

static int ImagePanel_SetDrawColor(lua_State *L)
{
	Color color = luaL_checkcolor(L, 2);
	luaL_checkimagepanel(L, 1)->SetDrawColor(color);
	return 0;
}

static int ImagePanel_GetImage(lua_State *L)
{
	const char *imageName = luaL_checkimagepanel(L, 1)->GetImageName();
	if (imageName)
		lua_pushstring(L, imageName);
	else
		lua_pushnil(L);

	return 1;
}

static int ImagePanel___index(lua_State *L)
{
	ImagePanel *pImagePanel = lua_toimagepanel(L, 1);
	if (pImagePanel == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LImagePanel *plImagePanel = dynamic_cast<LImagePanel *>(pImagePanel);
	if (plImagePanel && plImagePanel->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plImagePanel->m_nTableReference);
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

static int ImagePanel___newindex(lua_State *L)
{
	ImagePanel *pImagePanel = lua_toimagepanel(L, 1);
	if (pImagePanel == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LImagePanel *plImagePanel = dynamic_cast<LImagePanel *>(pImagePanel);
	if (plImagePanel)
	{
		if (plImagePanel->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plImagePanel->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plImagePanel->m_nTableReference);
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

static int ImagePanel___gc(lua_State *L)
{
	LImagePanel *plImagePanel = dynamic_cast<LImagePanel *>(lua_toimagepanel(L, 1));
	if (plImagePanel)
	{
		--plImagePanel->m_nRefCount;
		if (plImagePanel->m_nRefCount <= 0)
		{
			delete plImagePanel;
		}
	}
	return 0;
}

static int ImagePanel___eq(lua_State *L)
{
	lua_pushboolean(L, lua_toimagepanel(L, 1) == lua_toimagepanel(L, 2));
	return 1;
}

static int ImagePanel___tostring(lua_State *L)
{
	ImagePanel *pImagePanel = lua_toimagepanel(L, 1);
	if (pImagePanel == NULL)
		lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char *pName = pImagePanel->GetName();
		if (Q_strcmp(pName, "") == 0)
			pName = "(no name)";
		lua_pushfstring(L, "ImagePanel: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg ImagePanelmeta[] =
{
	{"SetImage", ImagePanel_SetImage},
	{"SetShouldScaleImage", ImagePanel_SetShouldScaleImage},
	{"SetDrawColor", ImagePanel_SetDrawColor},
	{"GetImage", ImagePanel_GetImage},
	{"__index", ImagePanel___index},
	{"__newindex", ImagePanel___newindex},
	{"__gc", ImagePanel___gc},
	{"__eq", ImagePanel___eq},
	{"__tostring", ImagePanel___tostring},
	{NULL, NULL}
};

static int luasrc_ImagePanel(lua_State *L)
{
	ImagePanel *pImagePanel = new LImagePanel(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
											   luaL_optstring(L, 2, NULL),
											   L);
	lua_pushimagepanel(L, pImagePanel);
	return 1;
}

static const luaL_Reg ImagePanel_funcs[] =
{
	{"ImagePanel", luasrc_ImagePanel},
	{NULL, NULL}
};

/* open ImagePanel object */
LUALIB_API int luaopen_vgui_ImagePanel(lua_State *L)
{
	luaL_newmetatable(L, "ImagePanel");
	luaL_register(L, NULL, ImagePanelmeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", ImagePanel_funcs);
	lua_pop(L, 2);
	return 1;
}
