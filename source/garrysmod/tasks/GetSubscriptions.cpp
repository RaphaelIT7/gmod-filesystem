#include "garrysmod/public/IAddonDownloadNotify.h"
#include "Tasks.h"

void Addon::Task::GetSubscriptions::Start()
{
	m_bFinished = false;
	m_bReady = false;
	m_Items.clear();

	m_pAddonSystem->Clear();

	UGCQueryHandle_t handle = SteamUGC()->CreateQueryUserUGCRequest(
		SteamUser()->GetSteamID().GetAccountID(),
		k_EUserUGCList_Subscribed,
		k_EUGCMatchingUGCType_Items,
		k_EUserUGCListSortOrder_SubscriptionDateDesc,
		4000,
		4000,
		1
	);

	SteamAPICall_t hCall = SteamUGC()->SendQueryUGCRequest(handle);
	m_Query.Set( hCall, this, &Addon::Task::GetSubscriptions::OnQueryCompleted );
}

void Addon::Task::GetSubscriptions::CheckForWastedSpace()
{
	// ToDo
}

void Addon::Task::GetSubscriptions::OnQueryCompleted( SteamUGCQueryCompleted_t *pResult, bool bIOFailure )
{
	m_bReady = true;
	if ( bIOFailure || pResult->m_eResult != k_EResultOK )
		return;

	for ( uint32 i = 0; i < pResult->m_unNumResultsReturned; ++i )
	{
		SteamUGCDetails_t details;
		if ( SteamUGC()->GetQueryUGCResult( pResult->m_handle, i, &details ) )
		{
			m_Items.push_back( details );
			m_pAddonSystem->AddAddonFromSteamDetails( details );
		}
	}

	SteamUGC()->ReleaseQueryUGCRequest( pResult->m_handle );
}

void Addon::Task::GetSubscriptions::Cycle()
{
	if ( !m_bReady )
	{
		if ( m_pAddonSystem->Notify() )
			m_pAddonSystem->Notify()->SubscriptionsProgress( (int)m_Items.size(), (int)SteamUGC()->GetNumSubscribedItems() );

		return;
	}

	CheckForWastedSpace();

	m_bFinished = true;
	if ( m_pAddonSystem->Notify() )
		m_pAddonSystem->Notify()->Finish();
}

bool Addon::Task::GetSubscriptions::Finished()
{
	return m_bFinished;
}