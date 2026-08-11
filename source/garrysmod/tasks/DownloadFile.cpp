#include "Tasks.h"

class ThreadedUGCAccess : public Bootil::Threads::Thread
{
public:
	~ThreadedUGCAccess() override = default;
	void Run() override;
};

void ThreadedUGCAccess::Run()
{
}

Addon::Task::DownloadFile::DownloadFile( const IAddonSystem::Information &info ) : m_downloadCallback( this, &Addon::Task::DownloadFile::OnItemDownloaded )
{
	m_info = info;
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
}

void Addon::Task::DownloadFile::Cycle()
{
}

bool Addon::Task::DownloadFile::Finished()
{
	return m_bIsFinished;
}
