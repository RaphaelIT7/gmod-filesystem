#include "garrysmod/AddonFileSystem.h"
#include "garrysmod/DedicatedServerAddons.h"
#include "garrysmod/public/IAddonDownloadNotify.h"
#include "garrysmod/public/IGet.h"
#include "garrysmod/tasks/Tasks.h"
#include "tier1/keyvalues.h"
#include "sdk_backports.h"
#include "filesystem.h"
#include <charconv>

void Addon::FileSystem::Clear()
{
	Msg( "Addon::FileSystem::Clear\n" );
}

void Addon::FileSystem::Refresh()
{
	Msg( "Addon::FileSystem::Refresh\n" );

	UpdateModPath();
	Load();
	MarkChanged();
}

// RaphaelIT7 (ToDo):
// It currently crashes after the last call made here, idk why.
bool Addon::FileSystem::MountFile( const std::string& gmaPath, std::vector<std::string>* files, uint64_t wsid, uint64_t wsid2, IAddonSystem::AddonSource unknown )
{
	Msg( "Addon::FileSystem::MountFile\n" );
	return false;
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
	Msg( "CAddonFileSystem::Save\n" );
	KeyValues* kv = new KeyValues( "addonnomount" );
	RunCodeAtScopeExit( kv->deleteThis(); );

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
	if (!get)
	{
		Error( "SFS: !get" );
		return;
	}

	if ( get->IsDedicatedServer() )
	{
		Warning( "   ^-- TODO: GarrysMod::DedicatedServer::RunAddonProcess!\n" );
		GarrysMod::DedicatedServer::RunAddonProcess( unknown1 );
		MountFloatingAddons();
		return;
	}
	else
	{
		m_CallbackSubscribed.Register( this, &Addon::FileSystem::OnRemoteStoragePublishedFileSubscribed );
		m_CallbackUnsubscribed.Register( this, &Addon::FileSystem::OnRemoteStoragePublishedFileUnsubscribed );

		AddJob( new Addon::Task::AddFloatingAddons() );
		AddJob( new Addon::Task::GetSubscriptions() );
		AddJob( new Addon::Task::MountAvailable() );

		if ( !IsOfflineMode() )
			AddJob( new Addon::Task::DownloadAddons() );

		Think();
	}
}

void Addon::FileSystem::OnRemoteStoragePublishedFileSubscribed( RemoteStoragePublishedFileSubscribed_t *info )
{
	if ( IsOfflineMode() ) 
		return;

	if ( info->m_nAppID != SteamUtils()->GetAppID() ) 
		return;

	if ( !IsSubscribed( info->m_nPublishedFileId ) )
		AddJob( new Addon::Task::OnSubscribed( info->m_nPublishedFileId ) ); 
}

void Addon::FileSystem::OnRemoteStoragePublishedFileUnsubscribed( RemoteStoragePublishedFileUnsubscribed_t *info )
{

}


Addon::FileSystem::FileSystem() :
	m_CallbackSubscribed( this, &Addon::FileSystem::OnRemoteStoragePublishedFileSubscribed ),
	m_CallbackUnsubscribed( this, &Addon::FileSystem::OnRemoteStoragePublishedFileUnsubscribed )
{
}

void Addon::FileSystem::UpdateModPath()
{
	m_strModPath = get->GameDir();
	m_strModPath.append( "\\workshop\\" );

	if ( false ) // fs_tellmeyoursecrets.GetBool()
		Msg( "Addon[UpdateModPath]: ModPath [%s]\n", m_strModPath.c_str() );

	Bootil::String::File::FixSlashes( m_strModPath );
	Bootil::String::Lower( m_strModPath );

	if ( false ) // fs_tellmeyoursecrets.GetBool()
		Msg( "Addon[UpdateModPath]: Cleaned [%s]\n", m_strModPath.c_str() );
}

bool Addon::FileSystem::IsOfflineMode()
{
	if ( get->IsDedicatedServer() || !SteamUser() )
		return true;

	return !SteamUser()->BLoggedOn();
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

void Addon::FileSystem::SetDownloadNotify( IAddonDownloadNotification *pDownloadNotify )
{
	Msg( "CAddonFileSystem::SetDownloadNotify\n" );
	m_pDownloadNotify = pDownloadNotify;
}

IAddonDownloadNotification *Addon::FileSystem::Notify()
{
	Msg( "CAddonFileSystem::Notify\n" );
	return m_pDownloadNotify;
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
	AddJob( new Addon::Task::ClearUnusedGMAs );
}

std::string Addon::FileSystem::GetAddonFilepath( uint64 wsid, bool bGMAOnly )
{
	Msg( "Addon::FileSystem::GetAddonFilepath(%llu - %s)\n", wsid, bGMAOnly ? "true" : "false" );
	char szFolder[260];
	uint64 size;
	uint32 timestamp;
	if ( !get->SteamUGC()->GetItemInstallInfo( wsid, &size, szFolder, sizeof( szFolder ), &timestamp ) )
		return "";

	std::string strSearchPath = Bootil::String::Format::Print("%s/***.*", szFolder);

	FileFindHandle_t hFindHandle;
	RunCodeAtScopeExit( g_pFullFileSystem->FindClose( hFindHandle ); );
	const char *pszFileName = g_pFullFileSystem->FindFirstEx( strSearchPath.c_str(), nullptr, &hFindHandle );
	while ( pszFileName )
	{
		std::string strFileName = pszFileName;
		std::string ext = Bootil::String::File::GetFileExtension( strFileName );

		bool bValid = false;
		if (bGMAOnly)
		{
			if (ext == "gma")
				bValid = true;
		}
		else
		{
			if (ext == "gma" || ext == "dupe" || ext == "gms" || ext == "dem")
				bValid = true;
		}

		 if ( bValid )
			return Bootil::String::Format::Print( "%s/%s", szFolder, pszFileName );

		pszFileName = g_pFullFileSystem->FindNext( hFindHandle );
	}

	return "";
}

void Addon::FileSystem::UnmountAddon( uint64_t wsid, const char *pszReason )
{
	Msg( "Addon::FileSystem::UnmountAddon (%s)\n", pszReason );
}

void Addon::FileSystem::UnmountServerAddons()
{
	Msg( "Addon::FileSystem::UnmountServerAddons\n" );
}

std::string Addon::FileSystem::IsAddonValidPreInstall( SteamUGCDetails_t details )
{
	Msg( "Addon::FileSystem::IsAddonValidPreInstall\n" );
	if ( details.m_bBanned )
		return "Addon is banned";

	if ( details.m_eFileType != k_EWorkshopFileTypeCommunity )
		return "Bad workshop file type";

	if ( details.m_rgchTitle[0] == '\0' )
		return "Addon is hidden, banned or doesn't exist";

	if ( details.m_nConsumerAppID != 4000 )
		return "Bad consumer AppID";

	constexpr int LEGACY_CHECK = 0x5E3351E1;
	if ( ( SteamUGC()->GetItemState( details.m_nPublishedFileId ) & k_EItemStateLegacyItem ) != 0 &&
		details.m_rtimeCreated >= LEGACY_CHECK )
	{
		return "Addon too new to use old API";
	}

	return "";
}

bool Addon::FileSystem::AllJobsFinished()
{
	Msg( "Addon::FileSystem::AllJobsFinished\n" );
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
	Msg( "Addon::FileSystem::GetSubList\n" );
	return m_Subscriptions;
}

void Addon::FileSystem::MountFloatingAddons()
{
	Msg( "Addon::FileSystem::MountFloatingAddons\n" );

	FileFindHandle_t hFindHandle;
	// RaphaelIT7:
	// GMod checks if the gma is inside garrysmod/
	// I dunno why they then just search in MOD...
	const char* pszFile = g_pFullFileSystem->FindFirstEx( "*.gma", "GAME", &hFindHandle );
	while ( pszFile )
	{
		// ToDo
		pszFile = g_pFullFileSystem->FindNext( hFindHandle );
	}

	g_pFullFileSystem->FindClose( hFindHandle );
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
	MarkChanged();
}

void Addon::FileSystem::OnAddonDownloadFailed( const IAddonSystem::Information &info )
{
	Msg( "Addon::FileSystem::OnAddonDownloadFailed\n" );
}

void Addon::FileSystem::Load()
{
}