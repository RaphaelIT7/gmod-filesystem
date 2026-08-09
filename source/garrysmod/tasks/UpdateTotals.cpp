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

Addon::Task::UpdateTotals::UpdateTotals(uint32 completed, uint32 total) {
	m_Completed = completed; 
	m_Total = total; 
}