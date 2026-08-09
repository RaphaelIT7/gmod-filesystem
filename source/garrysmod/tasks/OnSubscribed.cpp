#include "Tasks.h"

void Addon::Task::OnSubscribed::Start()
{
}

void Addon::Task::OnSubscribed::Cycle()
{
}

bool Addon::Task::OnSubscribed::Finished()
{
	return true;
}

Addon::Task::OnSubscribed::OnSubscribed( uint64_t wsid ) : m_WSID( wsid )
{
}
