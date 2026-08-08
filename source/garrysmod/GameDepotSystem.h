#pragma once

#include "public/IGameDepotSystem.h"

namespace GameDepot
{

class System : public IGameDepotSystem
{
public: // IGameDepotSystem
	void Refresh() override;
	void Clear() override;
	void Save() override;
	void SetMount(uint32_t param_1, bool param_2) override;
	void MarkGameAsMounted(const std::string)  override;
	const std::list<IGameDepotSystem::Information>& GetList() override;
	int GetRefreshCount() override;

public: // System
	System();
	void FindGame(std::string* param_1);
	void MountAsSteampipe(Information* param_1, bool param_2);
	void Mount(Information* param_1, bool param_2);
	void MountAsFallback(Information* param_1);
	void Load();
	void Setup();
};

}