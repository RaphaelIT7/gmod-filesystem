#pragma once

#include "public/IGameDepotSystem.h"

class GameDepot_System : public IGameDepotSystem {
	virtual void Refresh() override;
	virtual void Clear() override;
	virtual void Save() override;
	virtual void SetMount(uint param_1, bool param_2) override;
	virtual void MarkGameAsMounted(const std::string)  override;
	virtual const std::list<IGameDepotSystem::Information>& GetList() override;
	virtual void MountAsMapFix(uint32_t param_1) override;
	virtual void MountCurrentGame(const std::string& param_1) override;

	GameDepot_System(); // size[30]
	void FindGame(std::string* param_1); // size[93]
	void MountAsSteampipe(Information* param_1, bool param_2); // size[10]
	void Mount(Information* param_1, bool param_2); // size[48]
	void MountAsFallback(Information* param_1); // size[79]
	void Load(); // size[317]
	void Setup(); // size[87]
};