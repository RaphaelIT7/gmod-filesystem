#include "garrysmod/AddonFileSystem.h"
#include "garrysmod/public/IAddonDownloadNotify.h"
#include "garrysmod/public/IGet.h"
#include "tier1/keyvalues.h"
#include "filesystem.h"
#include <charconv>

void Addon::FileSystem::Clear()
{
	Msg( "Addon::FileSystem::Clear\n" );
}

void Addon::FileSystem::Refresh()
{
	Msg( "Addon::FileSystem::Refresh\n" );

	Load();
	MarkChanged();
}

int Addon::FileSystem::MountFile( const std::string& gmaPath, std::vector<std::string>* files, uint64_t wsid, uint64_t wsid2, IAddonSystem::AddonSource unknown )
{
	Msg( "Addon::FileSystem::MountFile\n" );
	return 0;
}

bool Addon::FileSystem::ShouldMount( uint64_t wsid )
{
	Msg( "Addon::FileSystem::ShouldMount2 %llu\n", wsid );
	auto it = m_AddonNoMount.find( wsid );
	if ( it != m_AddonNoMount.end() )
		return false;

	for ( IAddonSystem::Information info : m_Addons )
	{
		if ( info.wsid == wsid )
			return true;
	}

	Msg( "CAddonFileSystem::ShouldMount Nope? %llu\n", wsid );

	return false;
}

void Addon::FileSystem::SetShouldMount( uint64_t wsid, bool bShouldMount )
{
	Msg( "Addon::FileSystem::SetShouldMount\n" );

	auto it = m_AddonNoMount.find( wsid );
	if (it != m_AddonNoMount.end())
	{
		if (bShouldMount)
			m_AddonNoMount.erase(it);
	} else if (!bShouldMount)
		m_AddonNoMount.insert( wsid );
}

void Addon::FileSystem::Save()
{
	KeyValues* kv = new KeyValues( "addonnomount" );
	KeyValues* list = kv->CreateNewKey();

	int i = 0;
	char szIDBuffer[21];
	char szWSIDBuffer[21];
	for ( auto wsid : m_AddonNoMount )
	{
		++i;
		auto [wsPtr, _] = std::to_chars( szWSIDBuffer, szWSIDBuffer + sizeof( szWSIDBuffer ) - 1, wsid );
		*wsPtr = '\0';

		auto [idPtr, _2] = std::to_chars( szIDBuffer, szIDBuffer + sizeof( szIDBuffer ) - 1, i );
		*idPtr = '\0';

		list->SetString( szIDBuffer, szWSIDBuffer );
	}

	kv->SaveToFile( g_pFullFileSystem, "cfg/addonnomount.txt", "DEFAULT_WRITE_PATH" );
	kv->deleteThis();

	Msg( "CAddonFileSystem::Save\n" );
}

const std::list<IAddonSystem::Information>& Addon::FileSystem::GetList( ) const
{
	Msg( "CAddonFileSystem::GetList\n" );

	return m_Addons;
}

const std::list<IAddonSystem::UGCInfo>& Addon::FileSystem::GetUGCList( ) const
{
	Msg( "CAddonFileSystem::GetUGCList\n" );
	return m_UgcAddons;
}

void Addon::FileSystem::ScanForSubscriptions( const char *unknown1, bool unknown2 ) // NOTE: Gmod uses the Steamworks 1.57. The sourcesdk-minimal was outdated.
{
	Msg( "CAddonFileSystem::ScanForSubscriptions (%s)\n", unknown1 );
}

void Addon::FileSystem::Think()
{
	//Msg( "CAddonFileSystem::Think\n" );
	get->RunSteamCallbacks();

	if (m_bChanged)
	{
		m_bChanged = false;

		if (m_pDownloadNotify)
			m_pDownloadNotify->NotifySubscriptionChanges();
	}
}

void Addon::FileSystem::SetDownloadNotify( IAddonDownloadNotification* pDownloadNotify )
{
	Msg( "CAddonFileSystem::SetDownloadNotify\n" );
	m_pDownloadNotify = pDownloadNotify;
}

int Addon::FileSystem::Notify()
{
	Msg( "CAddonFileSystem::Notify\n" );
	return 0;
}

bool Addon::FileSystem::IsSubscribed( uint64_t wsid )
{
	Msg( "Addon::FileSystem::IsSubscribed %llu\n", wsid );
	for ( IAddonSystem::Information info : m_Addons )
	{
		if ( info.wsid == wsid )
			return true;
	}

	return false;
}

const IAddonSystem::Information* Addon::FileSystem::FindFileOwner( const std::string& )
{
	Msg( "Addon::FileSystem::FindFileOwner\n" );
	return nullptr;
}

void Addon::FileSystem::AddAddon( const IAddonSystem::Information &info )
{
	Msg( "Addon::FileSystem::AddAddon\n" );
}

void Addon::FileSystem::ClearUnusedGMAs()
{
	Msg( "Addon::FileSystem::ClearUnusedGMAs\n" );
}

const std::string& Addon::FileSystem::GetAddonFilepath( uint64_t wsid, bool )
{
	Msg( "Addon::FileSystem::GetAddonFilepath\n" );
	static std::string empty = "";
	return empty;
}

void Addon::FileSystem::UnmountAddon( uint64_t wsid, const char *pszUnknown )
{
	Msg( "Addon::FileSystem::UnmountAddon (%s)\n", pszUnknown );
}

void Addon::FileSystem::UnmountServerAddons()
{
	Msg( "Addon::FileSystem::UnmountServerAddons\n" );
}

void Addon::FileSystem::IsAddonValidPreInstall( SteamUGCDetails_t details )
{
	Msg( "Addon::FileSystem::IsAddonValidPreInstall\n" );
}

bool Addon::FileSystem::AllJobsFinished()
{
	// RaphaelIT7:
	// The GMod loading screen waits for this to return true
	// Else you will wait forever.
	return true;
}

void Addon::FileSystem::Shutdown()
{
	Msg( "Addon::FileSystem::Shutdown\n" );
}

void Addon::FileSystem::AddJob( Addon::Job::Base* base )
{
	Msg( "Addon::FileSystem::AddJob\n" );
}

const std::list<SteamUGCDetails_t>& Addon::FileSystem::GetSubList() const
{
	return m_Subscriptions;
}

void Addon::FileSystem::MountFloatingAddons()
{
	Msg( "Addon::FileSystem::MountFloatingAddons\n" );
}

void Addon::FileSystem::AddAddonFromSteamDetails( const SteamUGCDetails_t& )
{
	Msg( "Addon::FileSystem::AddAddonFromSteamDetails\n" );
}

void Addon::FileSystem::OnAddonSubscribed( const SteamUGCDetails_t& )
{
	Msg( "Addon::FileSystem::AddAddonFromSteamDetails\n" );
}

void Addon::FileSystem::AddUnloadedSubscription( uint64_t wsid )
{
	Msg( "Addon::FileSystem::AddAddonFromSteamDetails\n" );
}

void Addon::FileSystem::EnableLoadingUnloadedAddons()
{
}

bool Addon::FileSystem::HasChanges()
{
	Msg( "Addon::FileSystem::HasChanges\n" );
	return m_bChanged;
}

void Addon::FileSystem::MarkChanged()
{
	Msg( "Addon::FileSystem::MarkChanged\n" );
	m_bChanged = true;
}

void Addon::FileSystem::OnAddonDownloaded( const IAddonSystem::Information &info )
{
	Msg( "Addon::FileSystem::AddonDownloaded\n" );
}

void Addon::FileSystem::OnAddonDownloadFailed( const IAddonSystem::Information &info )
{
	Msg( "Addon::FileSystem::OnAddonDownloadFailed\n" );
}

void Addon::FileSystem::Load()
{
}