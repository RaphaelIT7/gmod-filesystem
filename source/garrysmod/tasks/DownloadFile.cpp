#include "garrysmod/public/IAddonDownloadNotify.h"
#include "garrysmod/public/IMenuSystem.h"
#include "garrysmod/public/IGet.h"
#include "Tasks.h"

void ThreadedUGCAccess::Run()
{
	uint64 nDiskSize = 0;
	uint32 nTimeStamp = 0;
	char szFolder[1024] = {};

	if ( !SteamUGC()->GetItemInstallInfo( m_ItemID, &nDiskSize, szFolder, sizeof(szFolder), &nTimeStamp ) )
	{
		Warning( "Threaded UGC Access failed for %llu (GetItemInstallInfo)\n", m_ItemID );
		WantsToClose();
		return;
	}

	if ( nDiskSize == 0 )
		nDiskSize = g_pFullFileSystem->Size( szFolder, "MOD" );

	if ( !m_Buffer.EnsureCapacity( nDiskSize ) )
		Warning( "Couldn't allocate memory for addon %llu (maybe %s is too big for Steam?)\n", m_ItemID, szFolder );

	// RaphaelIT7: Sooo... why exactly does GMod read the entire file???
	FileHandle_t hFileHandle = g_pFullFileSystem->Open( szFolder, "rb", "MOD" );
	if ( hFileHandle )
	{
		g_pFullFileSystem->Read( m_Buffer.GetBase(), nDiskSize, hFileHandle );
		g_pFullFileSystem->Close( hFileHandle );

		m_Buffer.SetWritten( nDiskSize );
		m_timestamp = nTimeStamp;
		m_bSuccess = true;
	}
	else
		Warning( "Failed to read addon file %s! Does it exist?\n", szFolder );

	WantsToClose();
}

Addon::Task::DownloadFile::DownloadFile( const IAddonSystem::Information &info )
	: m_downloadCallback( this, &Addon::Task::DownloadFile::OnItemDownloaded )
{
	m_Info = info;
	m_bIsFinished = true;
}

void Addon::Task::DownloadFile::OnItemDownloaded( DownloadItemResult_t *pResult )
{
	if ( pResult->m_unAppID != 4000 )
	{
		Warning( "OnItemDownloaded: invalid app id %i?\n", pResult->m_unAppID );
		return;
	}
}

void Addon::Task::DownloadFile::Start()
{
	const uint32 nItemState = SteamUGC()->GetItemState( m_Info.wsid );

	if ( ( nItemState & ( k_EItemStateSubscribed | k_EItemStateInstalled ) ) !=
		 ( k_EItemStateSubscribed | k_EItemStateInstalled ) || m_Info.canUpdate )
	{
		m_pAddonSystem->UnmountAddon( m_Info.wsid, "for update" );
		if ( SteamUGC()->DownloadItem( m_Info.wsid, true ) )
		{
			// RaphaelIT7:
			// GMod manually registers the callback m_downloadCallback here
			// BUUUT the newer CCallback class from Steam already takes care of that!

			if ( m_pAddonSystem->Notify() )
			{
				m_pAddonSystem->Notify()->Start();
				m_pAddonSystem->Notify()->StartDownload( m_Info.wsid, m_Info.hcontent_preview, m_Info.title.c_str(), m_Info.size );
			}
		}
		else
		{
			Warning( "Workshop: Failed to start Workshop Item download for '%s' (%llu)!\n", m_Info.title.c_str(), m_Info.wsid );
			NotifyFailed( "Failed to start addon download" );
		}
		return;
	}

	Bootil::String::File::FixSlashes( m_Info.file );

	ThreadedUGCAccess *pThread = new ThreadedUGCAccess;
	pThread->m_ItemID = m_Info.wsid;
	pThread->m_timestamp = 0;
	pThread->m_bSuccess = false;
	pThread->StartInThreadAndDetatch();

	m_pThread = pThread;
	m_bIsFinished = false;
	m_WarningTimer.Start();
}

void Addon::Task::DownloadFile::Cycle()
{
	if ( !m_pAddonSystem->Notify() )
		return;

	uint64_t nDownloadedBytes, nTotalBytes = 0;
	if ( SteamUGC()->GetItemDownloadInfo( m_Info.wsid, &nDownloadedBytes, &nTotalBytes ) )
	{
		m_pAddonSystem->Notify()->DownloadProgress( m_Info.wsid, m_Info.hcontent_preview, m_Info.title.c_str(), nDownloadedBytes, nTotalBytes );
		m_nBytesTransferred += nDownloadedBytes - m_nPrevDownloadedBytes;
		m_nPrevDownloadedBytes = nDownloadedBytes;
	}

	if ( m_WarningTimer.GetDurationInProgress().GetSeconds() > 10.0 )
	{
		if ( m_nBytesTransferred > 0x1FFF )
			m_iFailedCounter = 0;
		else
		{
			std::string strBytes = Bootil::String::Format::Memory( m_nBytesTransferred );
			Msg( "Transferred %s (%llu) in 10 seconds\n", strBytes.c_str(), m_nBytesTransferred );
			if ( ++m_iFailedCounter >= 5 )
			{
				Warning( "Cancelling workshop download %llu, it's too slow.. maybe stuck? Try again later.\n", m_Info.wsid );
				NotifyFailed( "Download was too slow" );
				return;
			}
		}

		m_WarningTimer.Start();
		m_nBytesTransferred = 0;
	}
}

bool Addon::Task::DownloadFile::Finished()
{
	return m_bIsFinished;
}

void Addon::Task::DownloadFile::NotifyFailed( const char *pszReason )
{
	m_pAddonSystem->OnAddonDownloadFailed( m_Info );

	m_bIsFinished = true;

	std::string strError = m_Info.title;
	strError.append( ";" ).append( pszReason );

	if ( get->MenuSystem() )
		get->MenuSystem()->SendProblemToMenu( "addon_download_failed", 2, strError.c_str() );
}