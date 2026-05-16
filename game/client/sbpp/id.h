//========== Copyright (C) 2026, Team HL2SB++, All rights reserved. ===========//
//
// Purpose: User IDs
//
//===========================================================================//

#ifndef ID_H
#define ID_H
#ifdef _WIN32
#pragma once
#endif

class CUserID
{
public:
	static CUserID &Get()
	{
		static CUserID instance;
		return instance;
	}

	CUserID();

	const char *GetID() const
	{
		return m_szID;
	}

	// TODO: is this even useful
	void Regenerate();

	void Init();

private:
	void GenerateID( char *out, int len );
	bool LoadFromFile();
	void SaveToFile();

private:
	char m_szID[33];
};

#endif
