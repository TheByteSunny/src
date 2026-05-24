//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lTextEntry.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LTextEntry::LTextEntry(Panel *parent, const char *panelName, const char *text, lua_State *L) : TextEntry(parent, panelName)
{
#if defined( LUA_SDK )
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif // LUA_SDK
	if (text)
		SetText(text);
}

LTextEntry::~LTextEntry()
{
#if defined( LUA_SDK )
	lua_unref( m_lua_State, m_nTableReference );
#endif // LUA_SDK
}


/*
** access functions (stack -> C)
*/

LUA_API lua_TextEntry *lua_totextentry(lua_State *L, int idx)
{
	PHandle *phPanel = dynamic_cast<PHandle *>((PHandle *)lua_touserdata(L, idx));
	if (phPanel == NULL)
		return NULL;
	return dynamic_cast<lua_TextEntry *>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushtextentry(lua_State *L, TextEntry *pTextEntry)
{
	LTextEntry *plTextEntry = dynamic_cast<LTextEntry *>(pTextEntry);
	if (plTextEntry)
		++plTextEntry->m_nRefCount;
	PHandle *phPanel = (PHandle *)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pTextEntry);
	luaL_getmetatable(L, "TextEntry");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_TextEntry *luaL_checktextentry(lua_State *L, int narg)
{
	lua_TextEntry *d = lua_totextentry(L, narg);
	if (d == NULL)
		luaL_argerror(L, narg, "TextEntry expected, got INVALID_PANEL");
	return d;
}

static int TextEntry_SetText(lua_State *L)
{
	luaL_checktextentry(L, 1)->SetText(luaL_checkstring(L, 2));
	return 0;
}

static int TextEntry_GetText(lua_State *L)
{
	char buf[1024];
	luaL_checktextentry(L, 1)->GetText(buf, sizeof(buf));
	lua_pushstring(L, buf);
	return 1;
}

static int TextEntry_GetTextLength(lua_State *L)
{
	lua_pushinteger(L, luaL_checktextentry(L, 1)->GetTextLength());
	return 1;
}

static int TextEntry_SetEditable(lua_State *L)
{
	luaL_checktextentry(L, 1)->SetEditable(luaL_checkboolean(L, 2));
	return 0;
}

static int TextEntry_IsEditable(lua_State *L)
{
	lua_pushboolean(L, luaL_checktextentry(L, 1)->IsEditable());
	return 1;
}

static int TextEntry_SelectAllOnFirstFocus(lua_State *L)
{
	luaL_checktextentry(L, 1)->SelectAllOnFirstFocus(luaL_checkboolean(L, 2));
	return 0;
}

static int TextEntry_HasFocus(lua_State *L)
{
	lua_pushboolean(L, luaL_checktextentry(L, 1)->HasFocus());
	return 1;
}

static int TextEntry___index(lua_State *L)
{
	TextEntry *pTextEntry = lua_totextentry(L, 1);
	if (pTextEntry == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LTextEntry *plTextEntry = dynamic_cast<LTextEntry *>(pTextEntry);
	if (plTextEntry && plTextEntry->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plTextEntry->m_nTableReference);
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

static int TextEntry___newindex(lua_State *L)
{
	TextEntry *pTextEntry = lua_totextentry(L, 1);
	if (pTextEntry == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LTextEntry *plTextEntry = dynamic_cast<LTextEntry *>(pTextEntry);
	if (plTextEntry)
	{
		if (plTextEntry->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plTextEntry->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plTextEntry->m_nTableReference);
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

static int TextEntry___gc(lua_State *L)
{
	LTextEntry *plTextEntry = dynamic_cast<LTextEntry *>(lua_totextentry(L, 1));
	if (plTextEntry)
	{
		--plTextEntry->m_nRefCount;
		if (plTextEntry->m_nRefCount <= 0)
		{
			delete plTextEntry;
		}
	}
	return 0;
}

static int TextEntry___eq(lua_State *L)
{
	lua_pushboolean(L, lua_totextentry(L, 1) == lua_totextentry(L, 2));
	return 1;
}

static int TextEntry___tostring(lua_State *L)
{
	TextEntry *pTextEntry = lua_totextentry(L, 1);
	if (pTextEntry == NULL)
		lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char *pName = pTextEntry->GetName();
		if (Q_strcmp(pName, "") == 0)
			pName = "(no name)";
		lua_pushfstring(L, "TextEntry: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg TextEntrymeta[] =
{
	{"SetText", TextEntry_SetText},
	{"GetText", TextEntry_GetText},
	{"GetTextLength", TextEntry_GetTextLength},
	{"SetEditable", TextEntry_SetEditable},
	{"IsEditable", TextEntry_IsEditable},
	{"SelectAllOnFirstFocus", TextEntry_SelectAllOnFirstFocus},
	{"HasFocus", TextEntry_HasFocus},
	{"__index", TextEntry___index},
	{"__newindex", TextEntry___newindex},
	{"__gc", TextEntry___gc},
	{"__eq", TextEntry___eq},
	{"__tostring", TextEntry___tostring},
	{NULL, NULL}
};

static int luasrc_TextEntry(lua_State *L)
{
	TextEntry *pTextEntry = new LTextEntry(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
										   luaL_optstring(L, 2, NULL),
										   luaL_optstring(L, 3, ""),
										   L);
	lua_pushtextentry(L, pTextEntry);
	return 1;
}

static const luaL_Reg TextEntry_funcs[] =
{
	{"TextEntry", luasrc_TextEntry},
	{NULL, NULL}
};

/* open TextEntry object */
LUALIB_API int luaopen_vgui_TextEntry(lua_State *L)
{
	luaL_newmetatable(L, "TextEntry");
	luaL_register(L, NULL, TextEntrymeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", TextEntry_funcs);
	lua_pop(L, 2);
	return 1;
}
