#include "Tasks.h"

Addon::Task::DownloadAddons::DownloadAddons( bool bMountAfter ) : m_bMountAfter( bMountAfter )
{
}

void Addon::Task::DownloadAddons::Start()
{
    size_t downloadCount = 0;
    for ( auto &addon : m_pAddonSystem->GetList() )
    {
        if ( !addon.downloaded || addon.canUpdate )
            ++downloadCount;
    }

    if ( downloadCount == 0 )
    {
        if ( m_bMountAfter )
            m_pAddonSystem->AddJob( new Task::MountAvailable() );

        return;
    }

    std::size_t remaining = downloadCount;
    for ( auto &addon : m_pAddonSystem->GetList() )
    {
        if ( addon.downloaded && !addon.canUpdate )
            continue;

        m_pAddonSystem->AddJob( new Task::UpdateTotals( --remaining, downloadCount ) );
        m_pAddonSystem->AddJob( new Task::DownloadFile( addon ) );
    }

    m_pAddonSystem->AddJob(new Task::MountAvailable());
}

void Addon::Task::DownloadAddons::Cycle()
{
}

bool Addon::Task::DownloadAddons::Finished()
{
	return true;
}