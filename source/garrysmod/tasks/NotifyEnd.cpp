#include "garrysmod/public/IAddonDownloadNotify.h"
#include "Tasks.h"

void Addon::Task::NotifyEnd::Start()
{
	if ( m_pAddonSystem->Notify() )
		m_pAddonSystem->Notify()->Finish();
}

void Addon::Task::NotifyEnd::Cycle()
{
}

bool Addon::Task::NotifyEnd::Finished()
{
	return true;
}