#include "garrysmod/public/IAddonDownloadNotify.h"
#include "garrysmod/public/IGamemodeSystem.h"
#include "Tasks.h"

void Addon::Task::MountAvailable::Start()
{
	if ( m_pAddonSystem->Notify() )
		m_pAddonSystem->Notify()->SendMessage( "#ugc.mounting" );
}

void Addon::Task::MountAvailable::Cycle()
{
	if ( m_nCycle <= 1 )
	{
		++m_nCycle;
		return;
	}

	Msg( "Addons have changes - remounting\n" );
	m_pAddonSystem->Clear();
	m_pAddonSystem->Refresh();
	
	g_pFullFileSystem->Gamemodes()->Refresh();
	g_pFullFileSystem->DoFilesystemRefresh();

	if ( m_pAddonSystem->Notify() )
		m_pAddonSystem->Notify()->Finish();

	m_bFinished = true;
}

bool Addon::Task::MountAvailable::Finished()
{
	return m_bFinished;
}