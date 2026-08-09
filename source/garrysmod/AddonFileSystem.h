#pragma once

#include "garrysmod/public/IAddonSystem.h"
#include "steam/isteamugc.h"
#include "unordered_stuff.h"

namespace Addon
{

class FileSystem : public IAddonSystem
{
public: // IAddonSystem
	void Clear( ) override;
	void Refresh( ) override;
	int MountFile( const std::string& gmaPath, std::vector<std::string>* files, uint64_t wsid, uint64_t wsid2, IAddonSystem::AddonSource ) override;
	bool ShouldMount( uint64_t wsid ) override;
	void SetShouldMount( uint64_t wsid, bool bShouldMount ) override;
	void Save( ) override;
	const std::list<IAddonSystem::Information> &GetList( ) const override;
	const std::list<IAddonSystem::UGCInfo> &GetUGCList( ) const override;
	void ScanForSubscriptions( const char *, bool ) override;
	void Think( ) override;
	void SetDownloadNotify( IAddonDownloadNotification *pDownloadNotify ) override;
	int Notify( ) override;
	bool IsSubscribed( uint64_t wsid ) override;
	const IAddonSystem::Information *FindFileOwner( const std::string & ) override;
	void AddAddon( const IAddonSystem::Information & ) override;
	void ClearUnusedGMAs( ) override;
	const std::string& GetAddonFilepath( uint64_t wsid, bool ) override;
	void UnmountAddon( uint64_t wsid, const char* ) override;
	void UnmountServerAddons( ) override;
	void IsAddonValidPreInstall( SteamUGCDetails_t ) override;
	bool AllJobsFinished() override;
	void Shutdown( ) override;
	void AddJob( Addon::Job::Base * ) override;
	const std::list<SteamUGCDetails_t> &GetSubList( ) const override;
	void MountFloatingAddons( ) override;
	void AddAddonFromSteamDetails( const SteamUGCDetails_t & ) override;
	void OnAddonSubscribed( const SteamUGCDetails_t & ) override;
	void AddUnloadedSubscription( uint64_t ) override;
	void EnableLoadingUnloadedAddons() override;
	bool HasChanges( ) override;
	void MarkChanged( ) override;
	void OnAddonDownloaded( const IAddonSystem::Information & ) override;
	void OnAddonDownloadFailed( const IAddonSystem::Information & ) override;
	void Load( ) override;

public: // FileSystem

	bool IsOfflineMode();
	void OnRemoteStoragePublishedFileSubscribed(RemoteStoragePublishedFileSubscribed_t* info);
	void OnRemoteStoragePublishedFileUnsubscribed(RemoteStoragePublishedFileUnsubscribed_t* info);

private:
	bool m_bChanged = false;
	std::list<IAddonSystem::Information> m_Addons;
	std::list<IAddonSystem::UGCInfo> m_UgcAddons;
	std::list<SteamUGCDetails_t> m_Subscriptions;
	unordered_set<uint64_t> m_AddonNoMount;
	IAddonDownloadNotification* m_pDownloadNotify = nullptr;
	CCallback<Addon::FileSystem, RemoteStoragePublishedFileSubscribed_t> m_CallbackSubscribed;
	CCallback<Addon::FileSystem, RemoteStoragePublishedFileUnsubscribed_t> m_CallbackUnsubscribed;
};

}