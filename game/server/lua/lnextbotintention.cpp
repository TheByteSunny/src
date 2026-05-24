//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "lnextbotintention.h"
#include "lnextbot.h"

CLuaNextBotIntention::CLuaNextBotIntention( INextBot *bot ) : IIntention( bot )
{
	m_behaviour = new Behavior< CLuaNextBot >( new CLuaBehaviour(), "LuaBotBehaviour" );
}

CLuaNextBotIntention::~CLuaNextBotIntention()
{
	delete m_behaviour;
}

void CLuaNextBotIntention::Reset( void )
{
	IIntention::Reset();
	delete m_behaviour;
	m_behaviour = new Behavior< CLuaNextBot >( new CLuaBehaviour(), "LuaBotBehaviour" );
}

void CLuaNextBotIntention::Update( void )
{
	// the cast is now valid (it's defined above)
	CLuaNextBot *bot = static_cast< CLuaNextBot * >( GetBot()->GetNextBotCombatCharacter() );
	if ( bot )
		m_behaviour->Update( bot, GetUpdateInterval() );
}
