#pragma once

#include "garrysmod/public/IGamemodeSystem.h"
#include <memory_resource>
#include "tier1/convar.h"

namespace Gamemode
{

class System : public IGamemodeSystem
{
public:
	void OnJoinServer( const std::string & ) override;
	void OnLeaveServer( ) override;
	void Refresh( ) override;
	void Clear( ) override;
	const IGamemodeSystem::Information &Active( ) override;
	const IGamemodeSystem::Information &FindByName( const std::string &gamemode ) override;
	void SetActive( const std::string &gamemode ) override;
	const std::list<IGamemodeSystem::Information> &GetList( ) const override;
	bool IsServerBlacklisted( char const* address, char const* hostname, char const* description, char const* gm, char const* map ) override;

public:
	System();

private:
	friend class CBaseFileSystem;

	bool ChangeGamemode( const std::string& strGamemode, bool bRestore );
	void AddGamemode( std::string strGamemode );

	// RaphaelIT7:
	// GMod just leaks the memory and ConVars.
	// We do better- we allocate them using the m_ConVarArena and
	// on refresh we unregister using m_ConVarIdentifier and then we clear out the m_ConVarArena
	std::pmr::monotonic_buffer_resource m_ConVarArena;
	CVarDLLIdentifier_t m_ConVarIdentifier = -1;

	const char* AllocString( const char* pszString )
	{
		size_t nLength = strlen( pszString );
		char* pData = (char*)m_ConVarArena.allocate( nLength + 1 );
		strncpy( pData, pszString, nLength );
		pData[ nLength ] = 0;

		return pData;
	}

	ConVar* AllocConVar()
	{
		return (ConVar*)m_ConVarArena.allocate( sizeof(ConVar) );
	}

	std::list<IGamemodeSystem::Information> m_Gamemodes;
	std::string m_strActive;

	// March: might be done differently, review
	IGamemodeSystem::Information m_NotFound;
};

}