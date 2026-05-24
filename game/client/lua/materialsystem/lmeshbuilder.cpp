//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose: Lua bindings for CMeshBuilder.
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "materialsystem/imesh.h"
#include "lmeshbuilder.h"
#include "limesh.h"
#include "mathlib/lvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


lua_MeshBuilderRef *luaL_checkmeshbuilderref( lua_State *L, int idx )
{
	lua_MeshBuilderRef *ref = (lua_MeshBuilderRef *)luaL_checkudata( L, idx, LUA_MESHBUILDERLIBNAME );
	if ( !ref || !ref->m_pBuilder )
		luaL_error( L, "CMeshBuilder is invalid" );

	return ref;
}

lua_MeshBuilderRef *luaL_checkmeshbuilder( lua_State *L, int idx, const char *fn )
{
	lua_MeshBuilderRef *ref = luaL_checkmeshbuilderref( L, idx );
	if ( !ref->m_bBuilding )
		luaL_error( L, "CMeshBuilder:%s called outside of Begin/End", fn );

	return ref;
}


static int CMeshBuilder_new( lua_State *L )
{
	lua_MeshBuilderRef *ref = (lua_MeshBuilderRef *)
		lua_newuserdata( L, sizeof( lua_MeshBuilderRef ) );
	ref->m_pBuilder  = new CMeshBuilder();
	ref->m_bBuilding = false;

	luaL_getmetatable( L, LUA_MESHBUILDERLIBNAME );
	lua_setmetatable( L, -2 );
	return 1;
}


static int CMeshBuilder_Begin( lua_State *L )
{
	lua_MeshBuilderRef *ref = luaL_checkmeshbuilderref( L, 1 );
	if ( ref->m_bBuilding )
		return luaL_error( L, "CMeshBuilder:Begin called while already building" );

	IMesh *pMesh = luaL_checkimesh( L, 2 );
	int    type  = luaL_checkint( L, 3 );

	int top = lua_gettop( L );
	if ( top >= 5 )
	{
		int vCount = luaL_checkint( L, 4 );
		int iCount = luaL_checkint( L, 5 );
		ref->m_pBuilder->Begin( pMesh, (MaterialPrimitiveType_t)type, vCount, iCount );
	}
	else
	{
		int numPrims = luaL_checkint( L, 4 );
		ref->m_pBuilder->Begin( pMesh, (MaterialPrimitiveType_t)type, numPrims );
	}

	ref->m_bBuilding = true;
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_End( lua_State *L )
{
	lua_MeshBuilderRef *ref = luaL_checkmeshbuilderref( L, 1 );
	if ( !ref->m_bBuilding )
		return 0; // tolerate End

	bool spew = lua_isboolean( L, 2 ) ? lua_toboolean( L, 2 ) != 0 : false;
	bool draw = lua_isboolean( L, 3 ) ? lua_toboolean( L, 3 ) != 0 : false;
	ref->m_pBuilder->End( spew, draw );
	ref->m_bBuilding = false;
	return 0;
}


static int CMeshBuilder_Reset( lua_State *L )
{
	lua_MeshBuilderRef *ref = luaL_checkmeshbuilder( L, 1, "Reset" );
	ref->m_pBuilder->Reset();
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_VertexCount( lua_State *L )
{
	lua_pushinteger( L, luaL_checkmeshbuilderref( L, 1 )->m_pBuilder->VertexCount() );
	return 1;
}

static int CMeshBuilder_IndexCount( lua_State *L )
{
	lua_pushinteger( L, luaL_checkmeshbuilderref( L, 1 )->m_pBuilder->IndexCount() );
	return 1;
}

static int CMeshBuilder_GetCurrentVertex( lua_State *L )
{
	lua_pushinteger( L, luaL_checkmeshbuilder( L, 1, "GetCurrentVertex" )->m_pBuilder->GetCurrentVertex() );
	return 1;
}

static int CMeshBuilder_GetCurrentIndex( lua_State *L )
{
	lua_pushinteger( L, luaL_checkmeshbuilder( L, 1, "GetCurrentIndex" )->m_pBuilder->GetCurrentIndex() );
	return 1;
}

static int CMeshBuilder_SelectVertex( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "SelectVertex" )->m_pBuilder->SelectVertex( luaL_checkint( L, 2 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_SelectIndex( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "SelectIndex" )->m_pBuilder->SelectIndex( luaL_checkint( L, 2 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_AdvanceVertex( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "AdvanceVertex" )->m_pBuilder->AdvanceVertex();
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_AdvanceVertices( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "AdvanceVertices" )->m_pBuilder->AdvanceVertices( luaL_checkint( L, 2 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_AdvanceIndex( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "AdvanceIndex" )->m_pBuilder->AdvanceIndex();
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_AdvanceIndices( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "AdvanceIndices" )->m_pBuilder->AdvanceIndices( luaL_checkint( L, 2 ) );
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_Position3f( lua_State *L )
{
	lua_MeshBuilderRef *ref = luaL_checkmeshbuilder( L, 1, "Position3f" );

	if ( lua_gettop( L ) == 2 )
	{
		Vector v = luaL_checkvector( L, 2 );
		ref->m_pBuilder->Position3f( v.x, v.y, v.z );
	}
	else
	{
		ref->m_pBuilder->Position3f(
			(float)luaL_checknumber( L, 2 ),
			(float)luaL_checknumber( L, 3 ),
			(float)luaL_checknumber( L, 4 ) );
	}
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_Normal3f( lua_State *L )
{
	lua_MeshBuilderRef *ref = luaL_checkmeshbuilder( L, 1, "Normal3f" );

	if ( lua_gettop( L ) == 2 )
	{
		Vector v = luaL_checkvector( L, 2 );
		ref->m_pBuilder->Normal3f( v.x, v.y, v.z );
	}
	else
	{
		ref->m_pBuilder->Normal3f(
			(float)luaL_checknumber( L, 2 ),
			(float)luaL_checknumber( L, 3 ),
			(float)luaL_checknumber( L, 4 ) );
	}
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_Color3f( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "Color3f" )->m_pBuilder->Color3f(
		(float)luaL_checknumber( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_Color4f( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "Color4f" )->m_pBuilder->Color4f(
		(float)luaL_checknumber( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ),
		(float)luaL_checknumber( L, 5 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_Color3ub( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "Color3ub" )->m_pBuilder->Color3ub(
		(unsigned char)luaL_checkint( L, 2 ),
		(unsigned char)luaL_checkint( L, 3 ),
		(unsigned char)luaL_checkint( L, 4 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_Color4ub( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "Color4ub" )->m_pBuilder->Color4ub(
		(unsigned char)luaL_checkint( L, 2 ),
		(unsigned char)luaL_checkint( L, 3 ),
		(unsigned char)luaL_checkint( L, 4 ),
		(unsigned char)luaL_checkint( L, 5 ) );
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_Specular3f( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "Specular3f" )->m_pBuilder->Specular3f(
		(float)luaL_checknumber( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_Specular4ub( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "Specular4ub" )->m_pBuilder->Specular4ub(
		(unsigned char)luaL_checkint( L, 2 ),
		(unsigned char)luaL_checkint( L, 3 ),
		(unsigned char)luaL_checkint( L, 4 ),
		(unsigned char)luaL_checkint( L, 5 ) );
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_TexCoord1f( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "TexCoord1f" )->m_pBuilder->TexCoord1f(
		luaL_checkint( L, 2 ),
		(float)luaL_checknumber( L, 3 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_TexCoord2f( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "TexCoord2f" )->m_pBuilder->TexCoord2f(
		luaL_checkint( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_TexCoord3f( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "TexCoord3f" )->m_pBuilder->TexCoord3f(
		luaL_checkint( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ),
		(float)luaL_checknumber( L, 5 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_TexCoord4f( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "TexCoord4f" )->m_pBuilder->TexCoord4f(
		luaL_checkint( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ),
		(float)luaL_checknumber( L, 5 ),
		(float)luaL_checknumber( L, 6 ) );
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_BoneWeight( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "BoneWeight" )->m_pBuilder->BoneWeight(
		luaL_checkint( L, 2 ),
		(float)luaL_checknumber( L, 3 ) );
	
	lua_settop( L, 1 );
	return 1;
}

static int CMeshBuilder_BoneMatrix( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "BoneMatrix" )->m_pBuilder->BoneMatrix(
		luaL_checkint( L, 2 ),
		luaL_checkint( L, 3 ) );
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_UserData( lua_State *L )
{
	lua_MeshBuilderRef *ref = luaL_checkmeshbuilder( L, 1, "UserData" );
	float data[4] = {
		(float)luaL_checknumber( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ),
		(float)luaL_checknumber( L, 5 ),
	};
	ref->m_pBuilder->UserData( data );
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_Index( lua_State *L )
{
	luaL_checkmeshbuilder( L, 1, "Index" )->m_pBuilder->Index(
		(unsigned short)luaL_checkint( L, 2 ) );
	
	lua_settop( L, 1 );
	return 1;
}


static int CMeshBuilder_gc( lua_State *L )
{
	lua_MeshBuilderRef *ref = (lua_MeshBuilderRef *)luaL_checkudata( L, 1, LUA_MESHBUILDERLIBNAME );
	if ( ref && ref->m_pBuilder )
	{
		if ( ref->m_bBuilding )
		{
			// don't leak a vertex buffer lock
			ref->m_pBuilder->End( false, false );
			ref->m_bBuilding = false;
		}
		delete ref->m_pBuilder;
		ref->m_pBuilder = NULL;
	}
	return 0;
}

static int CMeshBuilder_tostring( lua_State *L )
{
	lua_MeshBuilderRef *ref = (lua_MeshBuilderRef *)luaL_checkudata( L, 1, LUA_MESHBUILDERLIBNAME );
	lua_pushfstring( L, "CMeshBuilder: %p (%s)",
		ref->m_pBuilder,
		ref->m_bBuilding ? "building" : "idle" );
	return 1;
}


static const luaL_Reg CMeshBuilder_methods[] = {
	{"Begin",             CMeshBuilder_Begin},
	{"End",               CMeshBuilder_End},
	{"Reset",             CMeshBuilder_Reset},

	{"VertexCount",       CMeshBuilder_VertexCount},
	{"IndexCount",        CMeshBuilder_IndexCount},
	{"GetCurrentVertex",  CMeshBuilder_GetCurrentVertex},
	{"GetCurrentIndex",   CMeshBuilder_GetCurrentIndex},
	{"SelectVertex",      CMeshBuilder_SelectVertex},
	{"SelectIndex",       CMeshBuilder_SelectIndex},

	{"AdvanceVertex",     CMeshBuilder_AdvanceVertex},
	{"AdvanceVertices",   CMeshBuilder_AdvanceVertices},
	{"AdvanceIndex",      CMeshBuilder_AdvanceIndex},
	{"AdvanceIndices",    CMeshBuilder_AdvanceIndices},

	{"Position3f",        CMeshBuilder_Position3f},
	{"Normal3f",          CMeshBuilder_Normal3f},

	{"Color3f",           CMeshBuilder_Color3f},
	{"Color4f",           CMeshBuilder_Color4f},
	{"Color3ub",          CMeshBuilder_Color3ub},
	{"Color4ub",          CMeshBuilder_Color4ub},

	{"Specular3f",        CMeshBuilder_Specular3f},
	{"Specular4ub",       CMeshBuilder_Specular4ub},

	{"TexCoord1f",        CMeshBuilder_TexCoord1f},
	{"TexCoord2f",        CMeshBuilder_TexCoord2f},
	{"TexCoord3f",        CMeshBuilder_TexCoord3f},
	{"TexCoord4f",        CMeshBuilder_TexCoord4f},

	{"BoneWeight",        CMeshBuilder_BoneWeight},
	{"BoneMatrix",        CMeshBuilder_BoneMatrix},
	{"UserData",          CMeshBuilder_UserData},
	{"Index",             CMeshBuilder_Index},

	{NULL, NULL}
};


static const luaL_Reg CMeshBuilder_lib[] = {
	{"new", CMeshBuilder_new},
	{NULL, NULL}
};


/*
** Open CMeshBuilder object
*/
LUALIB_API int luaopen_CMeshBuilder( lua_State *L )
{
	luaL_newmetatable( L, LUA_MESHBUILDERLIBNAME );

	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	lua_pushcfunction( L, CMeshBuilder_tostring );
	lua_setfield( L, -2, "__tostring" );

	lua_pushcfunction( L, CMeshBuilder_gc );
	lua_setfield( L, -2, "__gc" );

	luaL_register( L, NULL, CMeshBuilder_methods );
	lua_pop( L, 1 );

	luaL_register( L, LUA_MESHBUILDERLIBNAME, CMeshBuilder_lib );
	return 1;
}
