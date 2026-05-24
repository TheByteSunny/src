//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "mount.h"
#include "filesystem.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar hl2_mounted( "hl2_mounted", "0", FCVAR_DEVELOPMENTONLY );
ConVar portal_mounted( "portal_mounted", "0", FCVAR_DEVELOPMENTONLY );
ConVar css_mounted( "css_mounted", "0", FCVAR_DEVELOPMENTONLY );
ConVar hl1_mounted( "hl1_mounted", "0", FCVAR_DEVELOPMENTONLY );
ConVar hl2mp_mounted( "hl2mp_mounted", "0", FCVAR_DEVELOPMENTONLY );
ConVar ep2_mounted( "ep2_mounted", "0", FCVAR_DEVELOPMENTONLY );
ConVar episodic_mounted( "episodic_mounted", "0", FCVAR_DEVELOPMENTONLY );
ConVar dod_mounted( "dod_mounted", "0", FCVAR_DEVELOPMENTONLY );

struct GameMount
{
	const char *name;
	ConVar	   *var;
};

static GameMount mounts[] = { { "hl2", &hl2_mounted }, { "cstrike", &css_mounted }, { "portal", &portal_mounted }, { "hl1", &hl1_mounted }, { "hl2mp", &hl2mp_mounted }, { "ep2", &ep2_mounted }, { "episodic", &episodic_mounted },
	{ "dod", &dod_mounted } };

static void ConcatPaths( char *dest, const char *basePath, const char *fileName, size_t destSize )
{
	snprintf( dest, destSize, "%s/%s", basePath, fileName );
}

static void StripDirFromFileName( char *fileName, size_t bufSize )
{
	size_t		 len = Q_strlen( fileName );
	const char	*suffix = "_dir.vpk";
	const size_t suffixLen = Q_strlen( suffix ); // 8
	if ( len > suffixLen && Q_stricmp( fileName + len - suffixLen, suffix ) == 0 )
	{
		fileName[len - suffixLen] = '\0';
		Q_strncat( fileName, ".vpk", bufSize );
	}
}

static void AddDirectoryAndVPks( const char *directoryPath )
{
	if ( !directoryPath || !directoryPath[0] )
		return;

	char resolvedPath[MAX_PATH];
	if ( !g_pFullFileSystem->RelativePathToFullPath( directoryPath, "GAME", resolvedPath, sizeof( resolvedPath ) ) )
	{
		if ( !g_pFullFileSystem->RelativePathToFullPath( directoryPath, NULL, resolvedPath, sizeof( resolvedPath ) ) )
			Q_strncpy( resolvedPath, directoryPath, sizeof( resolvedPath ) );
	}

	char dir[MAX_PATH];
	Q_strncpy( dir, resolvedPath, sizeof( dir ) );
	V_FixSlashes( dir );
	V_AppendSlash( dir, sizeof( dir ) );

	g_pFullFileSystem->AddSearchPath( dir, "GAME_MOUNT_TEMP" );

	FileFindHandle_t findHandle;
	const char		*fileName = g_pFullFileSystem->FindFirstEx( "*", "GAME_MOUNT_TEMP", &findHandle );
	if ( fileName )
	{
		do
		{
			size_t len = Q_strlen( fileName );
			if ( len > 8 && Q_stricmp( fileName + len - 8, "_dir.vpk" ) == 0 )
			{
				char vpkPath[MAX_PATH];
				Q_snprintf( vpkPath, sizeof( vpkPath ), "%s%s", dir, fileName );
				DevMsg( "Adding VPK: %s\n", vpkPath );
				g_pFullFileSystem->AddSearchPath( vpkPath, "GAME" );
			}
		} while ( ( fileName = g_pFullFileSystem->FindNext( findHandle ) ) );

		g_pFullFileSystem->FindClose( findHandle );
	}

	g_pFullFileSystem->AddSearchPath( dir, "GAME" );
	g_pFullFileSystem->RemoveSearchPath( dir, "GAME_MOUNT_TEMP" );
}

void loadMount()
{
	KeyValues *pKeyValues = new KeyValues( "mounts" );

	bool loaded = pKeyValues->LoadFromFile( g_pFullFileSystem, "cfg/mounts.kv", "GAME", true );
	if ( !loaded )
	{
		if ( !pKeyValues->LoadFromFile( g_pFullFileSystem, "cfg/mounts.kv", NULL, true ) )
		{
			pKeyValues->deleteThis();
			return;
		}
	}

	KeyValues *pSubKey = pKeyValues->GetFirstSubKey();

	while ( pSubKey )
	{
		const char *gameName = pSubKey->GetName();
		const char *path = pSubKey->GetString();

		if ( !path || !path[0] )
		{
			Warning( "loadMount: empty path for game %s in mounts.kv\n", gameName );
			pSubKey = pSubKey->GetNextKey();
			continue;
		}

		if ( !g_pFullFileSystem->IsDirectory( path, "GAME" ) && !g_pFullFileSystem->IsDirectory( path, NULL ) )
		{
			Warning( "Mount path does not exist: %s (game %s)\n", path, gameName );
			pSubKey = pSubKey->GetNextKey();
			continue;
		}

		for ( const auto &m : mounts )
		{
			if ( !Q_stricmp( gameName, m.name ) )
			{
				m.var->SetValue( 1 );
				break;
			}
		}

		Msg( "Mounting %s from %s\n", gameName, path );
		AddDirectoryAndVPks( path );

		pSubKey = pSubKey->GetNextKey();
	}

	pKeyValues->deleteThis();
}
