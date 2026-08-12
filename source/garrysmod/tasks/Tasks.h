#pragma once

#include "garrysmod/AddonFileSystem.h"
#include "Bootil/Bootil.h"
#include "filesystem.h"
#include "steam_api.h"

class ThreadedUGCAccess : public Bootil::Threads::Thread
{
public:
	~ThreadedUGCAccess() override = default;
	void Run() override;

public:
    uint64 m_ItemID;
    uint32 m_timestamp;
    Bootil::AutoBuffer m_Buffer;
    int m_bSuccess;
};

namespace Addon::Task
{

class AddFloatingAddons : public Addon::Job::Base
{
public:
	~AddFloatingAddons() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class ClearUnusedGMAs : public Addon::Job::Base
{
public:
	~ClearUnusedGMAs() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;

private:
	bool m_bIsFinished = false;
};

class DownloadAddons : public Addon::Job::Base
{
public:
	DownloadAddons( bool bMountAfter );
	~DownloadAddons() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;

private:
	bool m_bMountAfter;
};

class DownloadFile : public Addon::Job::Base
{
public:
	DownloadFile( const IAddonSystem::Information &info );
	~DownloadFile() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
	virtual void NotifyFailed( const char *pszReason );

private:
	IAddonSystem::Information m_Info;
	STEAM_CALLBACK( DownloadFile, OnItemDownloaded, DownloadItemResult_t, m_downloadCallback );
	bool m_bIsFinished = false;
	ThreadedUGCAccess* m_pThread = nullptr;
	CFastTimer m_WarningTimer;
	int m_iFailedCounter = 0;
	uint64_t m_nBytesTransferred = 0;
	uint64_t m_nPrevDownloadedBytes = 0;
};

class GetSubscriptions : public Addon::Job::Base
{
public:
	~GetSubscriptions() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;

private:
	void CheckForWastedSpace();
	void OnQueryCompleted( SteamUGCQueryCompleted_t *pResult, bool bIOFailure );
	CCallResult<GetSubscriptions, SteamUGCQueryCompleted_t> m_Query;

	std::vector<SteamUGCDetails_t> m_Items;
	bool m_bReady = false;
	bool m_bFinished = false;
};

class MountAvailable : public Addon::Job::Base
{
public:
	~MountAvailable() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;

private:
	int m_nCycle = 0;
	bool m_bFinished;
};

class NotifyStart : public Addon::Job::Base
{
public:
	~NotifyStart() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class NotifyEnd : public Addon::Job::Base
{
public:
	~NotifyEnd() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class OnSubscribed : public Addon::Job::Base
{
public:
	~OnSubscribed() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;

	OnSubscribed( uint64_t wsid );
private:
	uint64_t m_WSID;
};

class UpdateTotals : public Addon::Job::Base
{
public:
	~UpdateTotals() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
	UpdateTotals(uint32 completed, uint32 total);

	uint32_t m_Completed;
	uint32_t m_Total;
};

}