//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose: User IDs
//
//===========================================================================//

#include "cbase.h"
#include "id.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char *USERID_PATH = "cfg/id.txt";

static ConVar cl_userid( "cl_userid", "", FCVAR_ARCHIVE | FCVAR_HIDDEN | FCVAR_USERINFO, "Hidden install/user ID" );

CUserID::CUserID()
{
	m_szID[0] = '\0';
	Init();
}

void CUserID::Init()
{
	if ( !LoadFromFile() )
	{
		GenerateID( m_szID, sizeof( m_szID ) );
		SaveToFile();
	}

	cl_userid.SetValue( GetID() );
}

void CUserID::GenerateID( char *out, int len )
{
	if ( len < 33 )
		return;

	for ( int i = 0; i < 16; i++ )
	{
		int byte = RandomInt( 0, 255 );
		Q_snprintf( out + ( i * 2 ), len - ( i * 2 ), "%02x", byte );
	}

	out[32] = '\0';
}

bool CUserID::LoadFromFile()
{
	FileHandle_t f = filesystem->Open( USERID_PATH, "r" );

	if ( !f )
		return false;

	filesystem->Read( m_szID, sizeof( m_szID ) - 1, f );
	filesystem->Close( f );

	m_szID[32] = '\0';

	return ( m_szID[0] != '\0' );
}

void CUserID::SaveToFile()
{
	FileHandle_t f = filesystem->Open( USERID_PATH, "w" );

	if ( !f )
		return;

	filesystem->Write( m_szID, Q_strlen( m_szID ), f );
	filesystem->Close( f );
}

void CUserID::Regenerate()
{
	GenerateID( m_szID, sizeof( m_szID ) );
	SaveToFile();
}
