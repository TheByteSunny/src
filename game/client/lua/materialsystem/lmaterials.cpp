//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "materialsystem/limaterial.h"
#include "materialsystem/litexture.h"
#include "tier1/LKeyValues.h"
#include "limesh.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static int materials_FindMaterial( lua_State *L )
{
	const char *name = luaL_checkstring( L, 1 );
	const char *group = luaL_optstring( L, 2, TEXTURE_GROUP_OTHER );
	bool		complain = lua_isboolean( L, 3 ) ? lua_toboolean( L, 3 ) != 0 : true;
	IMaterial  *m = materials->FindMaterial( name, group, complain );
	lua_pushmaterial( L, m );
	return 1;
}

static int materials_CreateMaterial( lua_State *L )
{
	const char *name = luaL_checkstring( L, 1 );
	KeyValues  *pKV = luaL_checkkeyvalues( L, 2 );
	IMaterial  *pMat = materials->CreateMaterial( name, pKV );
	*(void **)lua_touserdata( L, 2 ) = NULL;
	lua_pushmaterial( L, pMat );
	return 1;
}

static int materials_IsMaterialLoaded( lua_State *L )
{
	// fixme
	const char *name = luaL_checkstring( L, 1 );
	IMaterial  *m = materials->FindMaterial( name, TEXTURE_GROUP_OTHER, false );
	lua_pushboolean( L, m && !m->IsErrorMaterial() );

	return 1;
}

static int materials_UncacheUnusedMaterials( lua_State *L )
{
	materials->UncacheUnusedMaterials();
	return 0;
}

static int materials_ReloadMaterials( lua_State *L )
{
	const char *sub = luaL_optstring( L, 1, NULL );
	materials->ReloadMaterials( sub );
	return 0;
}

static int materials_Bind( lua_State *L )
{
	IMaterial			*m = luaL_checkmaterial( L, 1 );
	CMatRenderContextPtr pRC( materials );
	pRC->Bind( m, NULL );
	return 0;
}

static int materials_SetColorModulation( lua_State *L )
{
	float				 r = (float)luaL_checknumber( L, 1 );
	float				 g = (float)luaL_checknumber( L, 2 );
	float				 b = (float)luaL_checknumber( L, 3 );
	CMatRenderContextPtr pRC( materials );
	float				 c[3] = { r, g, b };
	pRC->SetFloatRenderingParameter( 0, 0 );
	return 0;
}

static int materials_GetDynamicMesh( lua_State *L )
{
	bool	   buffered = lua_isboolean( L, 1 ) ? lua_toboolean( L, 1 ) != 0 : true;
	IMaterial *pMat = luaL_optmaterial( L, 2, NULL );

	CMatRenderContextPtr pRC( materials );
	IMesh				*m = pRC->GetDynamicMesh( buffered, NULL, NULL, pMat );
	lua_pushimesh( L, m, false ); // not owned
	return 1;
}

static int materials_CreateStaticMesh( lua_State *L )
{
	VertexFormat_t fmt = (VertexFormat_t)luaL_checknumber( L, 1 );
	const char	  *group = luaL_optstring( L, 2, TEXTURE_GROUP_OTHER );
	IMaterial	  *pMat = luaL_optmaterial( L, 3, NULL );

	CMatRenderContextPtr pRC( materials );
	IMesh				*m = pRC->CreateStaticMesh( fmt, group, pMat );
	lua_pushimesh( L, m, true ); // owned
	return 1;
}

static int materials_FindTexture( lua_State *L )
{
	const char *name = luaL_checkstring( L, 1 );
	const char *groupName = luaL_optstring( L, 2, TEXTURE_GROUP_OTHER );
	bool		complain = luaL_optboolean( L, 3, true );
	ITexture   *pTex = materials->FindTexture( name, groupName, complain );
	lua_pushtexture( L, pTex );
	return 1;
}

static int materials_IsTextureLoaded( lua_State *L )
{
	lua_pushboolean( L, materials->IsTextureLoaded( luaL_checkstring( L, 1 ) ) );
	return 1;
}

static int materials_CreateNamedRenderTargetTextureEx( lua_State *L )
{
	const char				   *name = luaL_checkstring( L, 1 );
	int							w = luaL_checkint( L, 2 );
	int							h = luaL_checkint( L, 3 );
	RenderTargetSizeMode_t		sizeMode = (RenderTargetSizeMode_t)luaL_checkint( L, 4 );
	ImageFormat					fmt = (ImageFormat)luaL_checkint( L, 5 );
	MaterialRenderTargetDepth_t depth = (MaterialRenderTargetDepth_t)luaL_optint( L, 6, MATERIAL_RT_DEPTH_SHARED );
	unsigned int				textureFlags = (unsigned int)luaL_optint( L, 7, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT );
	unsigned int				renderTargetFlags = (unsigned int)luaL_optint( L, 8, 0 );
	ITexture				   *pTex = materials->CreateNamedRenderTargetTextureEx( name, w, h, sizeMode, fmt, depth, textureFlags, renderTargetFlags );
	lua_pushtexture( L, pTex );
	return 1;
}

static int materials_CreateProceduralTexture( lua_State *L )
{
	const char *name = luaL_checkstring( L, 1 );
	const char *groupName = luaL_checkstring( L, 2 );
	int			w = luaL_checkint( L, 3 );
	int			h = luaL_checkint( L, 4 );
	ImageFormat fmt = (ImageFormat)luaL_checkint( L, 5 );
	int			flags = luaL_checkint( L, 6 );
	ITexture   *pTex = materials->CreateProceduralTexture( name, groupName, w, h, fmt, flags );
	lua_pushtexture( L, pTex );
	return 1;
}

static const luaL_Reg materialslib[] = {
	{"FindMaterial",            materials_FindMaterial},
	{"CreateMaterial",          materials_CreateMaterial},
	{"IsMaterialLoaded",        materials_IsMaterialLoaded},
	{"UncacheUnusedMaterials",  materials_UncacheUnusedMaterials},
	{"ReloadMaterials",         materials_ReloadMaterials},
	{"Bind",                    materials_Bind},
	{"SetColorModulation",      materials_SetColorModulation},

	{"FindTexture",                       materials_FindTexture},
	{"IsTextureLoaded",                   materials_IsTextureLoaded},
	{"CreateNamedRenderTargetTextureEx",  materials_CreateNamedRenderTargetTextureEx},
	{"CreateProceduralTexture",           materials_CreateProceduralTexture},

	// Mesh
    {"GetDynamicMesh",          materials_GetDynamicMesh},
    {"CreateStaticMesh",        materials_CreateStaticMesh},

	{NULL, NULL}
};


/*
** Open materials object
*/
LUALIB_API int luaopen_materials( lua_State *L )
{
	luaL_register( L, "materials", materialslib );
	return 1;
}
