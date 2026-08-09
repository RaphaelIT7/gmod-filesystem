#include "Tasks.h"

void Addon::Task::AddFloatingAddons::Start()
{
	m_pAddonSystem->MountFloatingAddons();
}

void Addon::Task::AddFloatingAddons::Cycle()
{
}

bool Addon::Task::AddFloatingAddons::Finished()
{
	return true;
}