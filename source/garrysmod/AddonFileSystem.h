#pragma once

#include "garrysmod/public/IAddonSystem.h"
#include "steam/isteamugc.h"
#include "unordered_stuff.h"
#include "AddonFileHandle.h"
#include <deque>
#include <map>

namespace Addon
{
enum class AddonType {
	Unknown = 0,
	Addon = 1,
	Dupe = 2,
	Save = 3,
	Demo = 4,
	ServerContent = 5,
};

struct MountedAddon
{
	std::string m_strPath;
	std::string m_strTitle;
	FileHandle_t m_hFileHandle;
	uint64_t m_nWsid;
	uint64_t m_nWsid2;
	bool m_bDeleteOnUnmount;
};

struct SearchFile
{
	std::string m_strFileName;
	bool		m_bFolder = false;
};

using Folder = std::map<std::string, Addon::FileInfo>;
using Folders = std::map<std::string, Folder>;

class FileSystem : public IAddonSystem
{
public: // IAddonSystem
	void Clear() override;
	void Refresh() override;
	bool MountFile( const std::string &gmaPath, std::vector<std::string> *files, uint64_t wsid, uint64_t wsid2, IAddonSystem::AddonSource source ) override;
	bool ShouldMount( uint64_t wsid ) override;
	bool SetShouldMount( uint64_t wsid, bool bShouldMount ) override;
	void Save() override;
	const std::list<IAddonSystem::Information> &GetList() const override;
	const std::list<IAddonSystem::UGCInfo> &GetUGCList() const override;
	void ScanForSubscriptions( const char *unknown1, bool unknown2 ) override;
	void Think() override;
	void SetDownloadNotify( IAddonDownloadNotification *pDownloadNotify ) override;
	IAddonDownloadNotification *Notify() override;
	bool IsSubscribed( uint64_t wsid ) override;
	const IAddonSystem::Information *FindFileOwner( const std::string &strFileName ) override;
	void AddAddon( const IAddonSystem::Information &info ) override;
	void ClearUnusedGMAs() override;
	std::string GetAddonFilepath( uint64_t wsid, bool bGMAOnly ) override;
	void UnmountAddon( uint64_t wsid, const char *pszReason ) override;
	void UnmountServerAddons() override;
	std::string IsAddonValidPreInstall( SteamUGCDetails_t details ) override;
	bool AllJobsFinished() override;
	void Shutdown() override;
	void AddJob( Addon::Job::Base *job ) override;
	const std::list<SteamUGCDetails_t> &GetSubList() const override;
	void MountFloatingAddons() override;
	void AddAddonFromSteamDetails( const SteamUGCDetails_t &details ) override;
	void OnAddonSubscribed( const SteamUGCDetails_t &details ) override;
	void AddUnloadedSubscription( uint64_t wsid ) override;
	void EnableLoadingUnloadedAddons() override;
	bool HasChanges() override;
	void MarkChanged() override;
	void OnAddonDownloaded( const IAddonSystem::Information &info ) override;
	void OnAddonDownloadFailed( const IAddonSystem::Information &info ) override;
	void Load() override;

public: // FileSystem
	FileSystem();
	void UpdateModPath();
	bool IsOfflineMode();
	void OnRemoteStoragePublishedFileSubscribed(RemoteStoragePublishedFileSubscribed_t* info);
	void OnRemoteStoragePublishedFileUnsubscribed(RemoteStoragePublishedFileUnsubscribed_t* info);
	Addon::AddonType GetAddonType(SteamUGCDetails_t details);
	void AddUGCFile(SteamUGCDetails_t details, Addon::AddonType type);
	Folder* GetFolder( const std::string &strPath, bool bCreate = false );
	void NormalizePath( std::string &strFileName );
	void SendUGCListUpdate();
	bool UnmountFile( std::string strFileName, const char *pszReason );
	bool MountAddon( IAddonSystem::Information &info );

	FileInfo *GetFile( std::string strFileName );
	FileHandle *GetFileEntry( std::string strFileName );

	void FindInAddon( const std::string &pPath, const std::string &pSearchPath, std::list<Addon::SearchFile> &results );
	void FindFirst( const std::string &pPath, std::list<Addon::SearchFile> &results, FileHandle_t hFileHandle );

	bool IsDirectory( std::string strFolderName );

private:
	bool m_bChanged = false;
	std::string m_strModPath;
	std::list<IAddonSystem::Information> m_Addons;
	std::list<IAddonSystem::UGCInfo> m_UgcAddons;
	std::list<SteamUGCDetails_t> m_Subscriptions;
	std::list<MountedAddon> m_MountedAddons;
	Folders m_Folders;
	unordered_set<uint64_t> m_AddonNoMount;
	IAddonDownloadNotification* m_pDownloadNotify = nullptr;
	std::deque<Addon::Job::Base*> m_Jobs;
	Addon::Job::Base *m_pCurrentJob = nullptr;
	CCallback<Addon::FileSystem, RemoteStoragePublishedFileSubscribed_t> m_CallbackSubscribed;
	CCallback<Addon::FileSystem, RemoteStoragePublishedFileUnsubscribed_t> m_CallbackUnsubscribed;
};

}