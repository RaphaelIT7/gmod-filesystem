#pragma once

#include "public/IGameDepotSystem.h"

namespace GameDepot
{
	extern bool bInitialized;
#ifndef DEDICATED
	std::string GetAppInstallDir_FixedCase( int nAppID );
	const char *DoMountDir( IGameDepotSystem::Information &info, const std::string &unk2, bool bToHead );
#endif

class System : public IGameDepotSystem
{
public: // IGameDepotSystem
	void Refresh() override;
	void Clear() override;
	void Save() override;
	void SetMount( uint32_t nAppID, bool bMounted ) override;
	void MarkGameAsMounted( const std::string strGameFolder ) override;
	const std::list<IGameDepotSystem::Information> &GetList() override;
	int GetRefreshCount() override;

public: // System
	System();
	void FindGame( std::string &strGameName );
	bool MountAsSteampipe( Information &info, bool bHead );
	void Mount( Information &info, bool bMount );
	void MountAsFallback( Information &info );
	void Load();
	void Setup();

	std::list<IGameDepotSystem::Information> m_MountedGames;
	int m_RefreshCount;
};

}