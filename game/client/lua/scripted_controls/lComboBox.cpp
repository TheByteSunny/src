//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lComboBox.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LComboBox::LComboBox(Panel* parent, const char* panelName, int numLines, bool allowEdit, lua_State* L) : ComboBox(parent, panelName, numLines, allowEdit)
{
#if defined(LUA_SDK)
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif
}

LComboBox::~LComboBox()
{
#if defined(LUA_SDK)
	lua_unref(m_lua_State, m_nTableReference);
#endif
}


/*
** access functions (stack -> C)
*/

LUA_API lua_ComboBox* lua_tocombobox(lua_State* L, int idx)
{
	PHandle* phPanel = dynamic_cast<PHandle*>((PHandle*)lua_touserdata(L, idx));
	if (phPanel == NULL) return NULL;
	return dynamic_cast<lua_ComboBox*>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushcombobox(lua_State* L, ComboBox* pComboBox)
{
	LComboBox* plComboBox = dynamic_cast<LComboBox*>(pComboBox);
	if (plComboBox) ++plComboBox->m_nRefCount;
	PHandle* phPanel = (PHandle*)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pComboBox);
	luaL_getmetatable(L, "ComboBox");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_ComboBox* luaL_checkcombobox(lua_State* L, int narg)
{
	lua_ComboBox* d = lua_tocombobox(L, narg);
	if (d == NULL) luaL_argerror(L, narg, "ComboBox expected, got INVALID_PANEL");
	return d;
}

static int ComboBox_AddItem(lua_State* L)
{
	luaL_checkcombobox(L, 1)->AddItem(luaL_checkstring(L, 2), NULL);
	return 0;
}

static int ComboBox_DeleteAllItems(lua_State* L)
{
	luaL_checkcombobox(L, 1)->DeleteAllItems();
	return 0;
}

static int ComboBox_GetActiveItem(lua_State* L)
{
	lua_pushinteger(L, luaL_checkcombobox(L, 1)->GetActiveItem());
	return 1;
}

static int ComboBox_SetText(lua_State* L)
{
	luaL_checkcombobox(L, 1)->SetText(luaL_checkstring(L, 2));
	return 0;
}

static int ComboBox_GetText(lua_State* L)
{
	char buf[1024];
	luaL_checkcombobox(L, 1)->GetText(buf, sizeof(buf));
	lua_pushstring(L, buf);
	return 1;
}

static int ComboBox_ActivateItem(lua_State* L)
{
	luaL_checkcombobox(L, 1)->ActivateItem(luaL_checkint(L, 2));
	return 0;
}

static int ComboBox___index(lua_State* L)
{
	ComboBox* pComboBox = lua_tocombobox(L, 1);
	if (pComboBox == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LComboBox* plComboBox = dynamic_cast<LComboBox*>(pComboBox);
	if (plComboBox && plComboBox->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plComboBox->m_nTableReference);
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

static int ComboBox___newindex(lua_State* L)
{
	ComboBox* pComboBox = lua_tocombobox(L, 1);
	if (pComboBox == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar1);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LComboBox* plComboBox = dynamic_cast<LComboBox*>(pComboBox);
	if (plComboBox)
	{
		if (plComboBox->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plComboBox->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plComboBox->m_nTableReference);
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

static int ComboBox___gc(lua_State* L)
{
	LComboBox* plComboBox = dynamic_cast<LComboBox*>(lua_tocombobox(L, 1));
	if (plComboBox)
	{
		--plComboBox->m_nRefCount;
		if (plComboBox->m_nRefCount <= 0) delete plComboBox;
	}
	return 0;
}

static int ComboBox___eq(lua_State* L)
{
	lua_pushboolean(L, lua_tocombobox(L, 1) == lua_tocombobox(L, 2));
	return 1;
}

static int ComboBox___tostring(lua_State* L)
{
	ComboBox* pComboBox = lua_tocombobox(L, 1);
	if (pComboBox == NULL) lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char* pName = pComboBox->GetName();
		if (Q_strcmp(pName, "") == 0) pName = "(no name)";
		lua_pushfstring(L, "ComboBox: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg ComboBoxMeta[] =
{
	{"AddItem", ComboBox_AddItem},
	{"DeleteAllItems", ComboBox_DeleteAllItems},
	{"GetActiveItem", ComboBox_GetActiveItem},
	{"SetText", ComboBox_SetText},
	{"GetText", ComboBox_GetText},
	{"ActivateItem", ComboBox_ActivateItem},
	{"__index", ComboBox___index},
	{"__newindex", ComboBox___newindex},
	{"__gc", ComboBox___gc},
	{"__eq", ComboBox___eq},
	{"__tostring", ComboBox___tostring},
	{NULL, NULL}
};

static int luasrc_ComboBox(lua_State* L)
{
	ComboBox* pComboBox = new LComboBox(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
										   luaL_checkstring(L, 2),
										   luaL_checkint(L, 3),
										   luaL_checkboolean(L, 4), L);
	lua_pushcombobox(L, pComboBox);
	return 1;
}

static const luaL_Reg ComboBox_funcs[] =
{
	{"ComboBox", luasrc_ComboBox},
	{NULL, NULL}
};

/* open ComboBox object */
LUALIB_API int luaopen_vgui_ComboBox(lua_State* L)
{
	luaL_newmetatable(L, "ComboBox");
	luaL_register(L, NULL, ComboBoxMeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", ComboBox_funcs);
	lua_pop(L, 2);
	return 1;
}
