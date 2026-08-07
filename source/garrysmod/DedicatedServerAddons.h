#pragma once

namespace GarrysMod
{

class DedicatedServer
{
public:
	// Called from DownloadAddon
	void MountAddon(SteamUGCDetails_t unknown1, std::string unknown2, bool unknown3);

	// Called from DownloadAddon
	void MountSteamUGCAddon(SteamUGCDetails_t unknown1, char* unknown2);

	// Called from RunAddonProcess
	void DownloadCollection();

	// Called from inside RunAddonProcess
	void LoadCachedAddonList();

	void RunChecks(const char* pszUnknown);

	void DownloadAddon();

	// Called from Addon::FileSystem::ScanForSubscriptions
	void RunAddonProcess(const char* pszUnknown);
}

}