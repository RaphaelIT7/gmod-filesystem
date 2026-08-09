#include "garrysmod/public/IAddonDownloadNotify.h"
#include "Tasks.h"

void Addon::Task::NotifyStart::Start()
{
	if ( m_pAddonSystem->Notify() )
		m_pAddonSystem->Notify()->Start();
}

void Addon::Task::NotifyStart::Cycle()
{
}

bool Addon::Task::NotifyStart::Finished()
{
	return true;
}