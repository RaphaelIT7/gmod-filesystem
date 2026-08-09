#include "Tasks.h"
#include "../public/IAddonDownloadNotify.h"

void Addon::Task::UpdateTotals::Start()
{
	if (m_pAddonSystem->Notify())
		m_pAddonSystem->Notify()->DownloadTotals(m_Completed, m_Total);
}

void Addon::Task::UpdateTotals::Cycle()
{
}

bool Addon::Task::UpdateTotals::Finished()
{
	return true;
}