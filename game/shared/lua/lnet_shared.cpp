//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "lnet_shared.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "mathlib/lvector.h"
#include "lbaseplayer_shared.h"
#include "lbaseentity_shared.h"
#include "utldict.h"

#ifdef CLIENT_DLL
#include "c_baseplayer.h"
#define CBasePlayer C_BasePlayer
#else
#include "player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bf_write *g_pLuaNetWriteBuf = NULL;
bf_read  *g_pLuaNetReadBuf  = NULL;


static CUtlDict< int, unsigned short > g_NetStrings; // name -> id
static CUtlVector< CUtlString >        g_NetStringsByID;

int LuaNet_GetStringID( const char *name )
{
	unsigned short i = g_NetStrings.Find( name );
	if ( i == g_NetStrings.InvalidIndex() )
		return -1;
	return g_NetStrings[ i ];
}

const char *LuaNet_GetStringName( int id )
{
	if ( id < 0 || id >= g_NetStringsByID.Count() )
		return NULL;
	return g_NetStringsByID[ id ].Get();
}

int LuaNet_AddString( const char *name )
{
	int existing = LuaNet_GetStringID( name );
	if ( existing != -1 )
		return existing;

	if ( g_NetStringsByID.Count() >= LUA_NET_STRING_TABLE_MAX )
	{
		Warning( "LuaNet: string table full (%d entries), cannot add '%s'\n",
				 LUA_NET_STRING_TABLE_MAX, name );
		return -1;
	}

	int id = g_NetStringsByID.Count();
	g_NetStringsByID.AddToTail( CUtlString( name ) );
	g_NetStrings.Insert( name, id );
	return id;
}


static bf_write *CheckWriteBuf( lua_State *L )
{
	if ( !g_pLuaNetWriteBuf )
		luaL_error( L, "net.Write* called outside of net.Start/Send pair" );
	return g_pLuaNetWriteBuf;
}

static bf_read *CheckReadBuf( lua_State *L )
{
	if ( !g_pLuaNetReadBuf )
		luaL_error( L, "net.Read* called outside of a net.Receive callback" );
	return g_pLuaNetReadBuf;
}


static int net_WriteInt( lua_State *L )
{
	int value = luaL_checkint( L, 1 );
	int bits  = luaL_checkint( L, 2 );
	if ( bits < 2 || bits > 32 )
		return luaL_error( L, "WriteInt: bits must be 2..32, got %d", bits );
	CheckWriteBuf( L )->WriteSBitLong( value, bits );
	return 0;
}

static int net_WriteUInt( lua_State *L )
{
	unsigned int value = (unsigned int)luaL_checknumber( L, 1 );
	int          bits  = luaL_checkint( L, 2 );
	if ( bits < 1 || bits > 32 )
		return luaL_error( L, "WriteUInt: bits must be 1..32, got %d", bits );
	CheckWriteBuf( L )->WriteUBitLong( value, bits );
	return 0;
}

static int net_WriteFloat( lua_State *L )
{
	CheckWriteBuf( L )->WriteFloat( (float)luaL_checknumber( L, 1 ) );
	return 0;
}

static int net_WriteDouble( lua_State *L )
{
	double v = luaL_checknumber( L, 1 );
	CheckWriteBuf( L )->WriteBytes( &v, sizeof( double ) );
	return 0;
}

static int net_WriteBool( lua_State *L )
{
	luaL_checktype( L, 1, LUA_TBOOLEAN );
	CheckWriteBuf( L )->WriteOneBit( lua_toboolean( L, 1 ) ? 1 : 0 );
	return 0;
}

static int net_WriteString( lua_State *L )
{
	const char *s = luaL_checkstring( L, 1 );
	CheckWriteBuf( L )->WriteString( s );
	return 0;
}

static int net_WriteData( lua_State *L )
{
	size_t      slen;
	const char *s   = luaL_checklstring( L, 1, &slen );
	int         len = luaL_optint( L, 2, (int)slen );
	if ( len < 0 || (size_t)len > slen )
		return luaL_error( L, "WriteData: invalid length %d (string is %d bytes)",
						   len, (int)slen );
	CheckWriteBuf( L )->WriteBytes( s, len );
	return 0;
}

static int net_WriteVector( lua_State *L )
{
	Vector v = luaL_checkvector( L, 1 );
	bf_write *buf = CheckWriteBuf( L );
	buf->WriteBitVec3Coord( v );
	return 0;
}

static int net_WriteAngle( lua_State *L )
{
	QAngle a = luaL_checkangle( L, 1 );
	bf_write *buf = CheckWriteBuf( L );
	buf->WriteBitAngle( a.x, 16 );
	buf->WriteBitAngle( a.y, 16 );
	buf->WriteBitAngle( a.z, 16 );
	return 0;
}

static int net_WriteEntity( lua_State *L )
{
	CBaseEntity *pEnt = lua_isnoneornil( L, 1 ) ? NULL : luaL_checkentity( L, 1 );
	int idx = pEnt ? pEnt->entindex() : 0;
	CheckWriteBuf( L )->WriteUBitLong( idx, MAX_EDICT_BITS );
	return 0;
}

static int net_ReadInt( lua_State *L )
{
	int bits = luaL_checkint( L, 1 );
	if ( bits < 2 || bits > 32 )
		return luaL_error( L, "ReadInt: bits must be 2..32" );
	lua_pushinteger( L, CheckReadBuf( L )->ReadSBitLong( bits ) );
	return 1;
}

static int net_ReadUInt( lua_State *L )
{
	int bits = luaL_checkint( L, 1 );
	if ( bits < 1 || bits > 32 )
		return luaL_error( L, "ReadUInt: bits must be 1..32" );
	lua_pushnumber( L, CheckReadBuf( L )->ReadUBitLong( bits ) );
	return 1;
}

static int net_ReadFloat( lua_State *L )
{
	lua_pushnumber( L, CheckReadBuf( L )->ReadFloat() );
	return 1;
}

static int net_ReadDouble( lua_State *L )
{
	double v = 0;
	CheckReadBuf( L )->ReadBytes( &v, sizeof( double ) );
	lua_pushnumber( L, v );
	return 1;
}

static int net_ReadBool( lua_State *L )
{
	lua_pushboolean( L, CheckReadBuf( L )->ReadOneBit() != 0 );
	return 1;
}

static int net_ReadString( lua_State *L )
{
	char buf[ 4096 ];
	CheckReadBuf( L )->ReadString( buf, sizeof( buf ) );
	lua_pushstring( L, buf );
	return 1;
}

static int net_ReadData( lua_State *L )
{
	int len = luaL_checkint( L, 1 );
	if ( len <= 0 || len > 65535 )
		return luaL_error( L, "ReadData: invalid length %d", len );
	char *buf = (char *)stackalloc( len );
	CheckReadBuf( L )->ReadBytes( buf, len );
	lua_pushlstring( L, buf, len );
	return 1;
}

static int net_ReadVector( lua_State *L )
{
	Vector v;
	CheckReadBuf( L )->ReadBitVec3Coord( v );
	lua_pushvector( L, v );
	return 1;
}

static int net_ReadAngle( lua_State *L )
{
	QAngle a;
	bf_read *buf = CheckReadBuf( L );
	a.x = buf->ReadBitAngle( 16 );
	a.y = buf->ReadBitAngle( 16 );
	a.z = buf->ReadBitAngle( 16 );
	lua_pushangle( L, a );
	return 1;
}

static int net_ReadEntity( lua_State *L )
{
	int idx = CheckReadBuf( L )->ReadUBitLong( MAX_EDICT_BITS );

#ifdef CLIENT_DLL
	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntity( idx );
#else
	CBaseEntity *pEnt = UTIL_EntityByIndex( idx );
#endif

	if ( pEnt )
		lua_pushentity( L, pEnt );
	else
		lua_pushnil( L );
	return 1;
}


static int net_BytesWritten( lua_State *L )
{
	if ( g_pLuaNetWriteBuf )
		lua_pushinteger( L, g_pLuaNetWriteBuf->GetNumBytesWritten() );
	else
		lua_pushinteger( L, 0 );
	return 1;
}

static int net_BitsWritten( lua_State *L )
{
	if ( g_pLuaNetWriteBuf )
		lua_pushinteger( L, g_pLuaNetWriteBuf->GetNumBitsWritten() );
	else
		lua_pushinteger( L, 0 );
	return 1;
}


static int net_Receive( lua_State *L )
{
	const char *name = luaL_checkstring( L, 1 );
	luaL_checktype( L, 2, LUA_TFUNCTION );

	lua_getfield( L, LUA_REGISTRYINDEX, "lua_net_receivers" );
	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 );
		lua_newtable( L );
		lua_pushvalue( L, -1 );
		lua_setfield( L, LUA_REGISTRYINDEX, "lua_net_receivers" );
	}

	lua_pushvalue( L, 2 );
	lua_setfield( L, -2, name );
	lua_pop( L, 1 );
	return 0;
}

void LuaNet_DispatchReceive( lua_State *L, const char *name, int bits, CBasePlayer *pSender )
{
	lua_getfield( L, LUA_REGISTRYINDEX, "lua_net_receivers" );
	if ( !lua_istable( L, -1 ) ) { lua_pop( L, 1 ); return; }

	lua_getfield( L, -1, name );
	if ( !lua_isfunction( L, -1 ) )
	{
		lua_pop( L, 2 );
		return;
	}

	lua_pushinteger( L, bits );
#ifndef CLIENT_DLL
	if ( pSender )
		lua_pushplayer( L, pSender );
	else
		lua_pushnil( L );
#else
	lua_pushnil( L );
#endif

	int args = 2;
	if ( lua_pcall( L, args, 0, 0 ) != 0 )
	{
		Warning( "LuaNet receive error in '%s': %s\n", name, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}

	lua_pop( L, 1 );
}

void LuaNet_SetString( int id, const char *name )
{
	while ( g_NetStringsByID.Count() <= id )
		g_NetStringsByID.AddToTail( CUtlString() );
	g_NetStringsByID[ id ] = name;
	g_NetStrings.Insert( name, id );
}

static const luaL_Reg net_funcs_shared[] = {
	{ "WriteInt",     net_WriteInt     },
	{ "WriteUInt",    net_WriteUInt    },
	{ "WriteFloat",   net_WriteFloat   },
	{ "WriteDouble",  net_WriteDouble  },
	{ "WriteBool",    net_WriteBool    },
	{ "WriteString",  net_WriteString  },
	{ "WriteData",    net_WriteData    },
	{ "WriteVector",  net_WriteVector  },
	{ "WriteAngle",   net_WriteAngle   },
	{ "WriteEntity",  net_WriteEntity  },

	{ "ReadInt",      net_ReadInt      },
	{ "ReadUInt",     net_ReadUInt     },
	{ "ReadFloat",    net_ReadFloat    },
	{ "ReadDouble",   net_ReadDouble   },
	{ "ReadBool",     net_ReadBool     },
	{ "ReadString",   net_ReadString   },
	{ "ReadData",     net_ReadData     },
	{ "ReadVector",   net_ReadVector   },
	{ "ReadAngle",    net_ReadAngle    },
	{ "ReadEntity",   net_ReadEntity   },

	{ "BytesWritten", net_BytesWritten },
	{ "BitsWritten",  net_BitsWritten  },

	{ "Receive",      net_Receive      },
	{ NULL, NULL }
};

// todo: should this really be in separate files?
extern int net_Start   ( lua_State *L );
extern int net_Send    ( lua_State *L );
extern int net_Broadcast( lua_State *L );
extern int net_SendToServer( lua_State *L );

LUALIB_API int luaopen_net( lua_State *L )
{
	luaL_register( L, LUA_NETLIBNAME, net_funcs_shared );

	lua_pushcfunction( L, net_Start );        lua_setfield( L, -2, "Start" );
#ifndef CLIENT_DLL
	lua_pushcfunction( L, net_Send );         lua_setfield( L, -2, "Send" );
	lua_pushcfunction( L, net_Broadcast );    lua_setfield( L, -2, "Broadcast" );
#else
	lua_pushcfunction( L, net_SendToServer ); lua_setfield( L, -2, "SendToServer" );
#endif
	lua_pop( L, 1 );

	return 0;
}
