#pragma once

#include "public/IGameDepotSystem.h"
#include "GameDepotSystem.h"
#include <stdlib.h>
#include <basetypes.h>
#include "tier0/dbg.h"
#include "tier1/keyvalues.h"
#include "filesystem.h"
#include <sdk_backports.h>
#include "public/IGet.h"
#include "steam_api.h"

IGameDepotSystem::Information mountableGames[] = {
	{ 220,     0, "Half-Life 2 & Episodes",            "hl2",               false, false, false, false, true,  true  },
	{ 240,     0, "Counter-Strike: Source",            "cstrike",           false, false, false, false, true,  true  },
	{ 440,     0, "Team Fortress 2",                   "tf",                false, false, false, false, true,  false },
	{ 300,     0, "Day of Defeat: Source",             "dod",               false, false, false, false, true,  false },
	{ 320,     0, "Half-Life 2: Deathmatch",           "hl2mp",             false, false, false, false, true,  false },
	{ 280,     0, "Half-Life: Source",                 "hl1",               false, false, false, false, true,  false },
	{ 360,     0, "Half-Life Deathmatch: Source",      "hl1mp",             false, false, false, false, true,  false },
	{ 22208,   0, "Zeno Clash (Model Pack)",           "zeno_clash",        false, false, false, false, false, false },
	{ 400,     0, "Portal",                            "portal",            false, false, false, false, true,  false },
	{ 17530,   0, "D.I.P.R.I.P.",                      "diprip",            false, false, false, false, false, false },
	{ 17500,   0, "Zombie Panic! Source",              "zps",               false, false, false, false, false, false },
	{ 17570,   0, "Pirates, Vikings and Knights II",   "pvkii",             false, false, false, false, false, false },
	{ 17580,   0, "Dystopia",                          "dystopia",          false, false, false, false, false, false },
	{ 17700,   0, "Insurgency (Source Mod)",           "insurgency",        false, false, false, false, false, false },
	{ 17510,   0, "Age of Chivalry",                   "ageofchivalry",     false, false, false, false, false, false },
	{ 550,     0, "Left 4 Dead 2",                     "left4dead2",        false, false, false, false, true,  false },
	{ 500,     0, "Left 4 Dead",                       "left4dead",         false, false, false, false, true,  false },
	{ 620,     0, "Portal 2",                          "portal2",           false, false, false, false, true,  false },
	{ 630,     0, "Alien Swarm",                       "swarm",             false, false, false, false, true,  false },
	{ 17710,   0, "Nuclear Dawn",                      "nucleardawn",       false, false, false, false, true,  false },
	{ 70000,   0, "Dino D-Day",                        "dinodday",          false, false, false, false, true,  false },
	{ 730,     0, "Counter-Strike: GO (Legacy Branch)","csgo",              false, false, false, false, true,  false },
	{ 225600,  0, "Blade Symphony",                    "berimbau",          false, false, false, false, true,  false },
	{ 251110,  0, "INFRA",                             "infra",             false, false, false, false, true,  false },
	{ 265630,  0, "Fistful of Frags",                  "fof",               false, false, false, false, true,  false },
	{ 221910,  0, "The Stanley Parable",               "thestanleyparable", false, false, false, false, false, false },
	{ 1224600, 0, "G String Game",                     "gstringv2",         false, false, false, false, true,  false },
	{ 1786950, 0, "Klaus Veen's Treason",              "treason",           false, false, false, false, true,  false },
	{ 1012110, 0, "Military Conflict: Vietnam",        "treason",           false, false, false, false, true,  false },
	{ 362890,  0, "Black Mesa",                        "bms",               false, false, false, false, true,  false },
};

void GameDepot::System::Refresh() {
	m_RefreshCount++;
	Clear();
	Load();
	Msg("GameDepot::System::Refresh()\n");
}

void GameDepot::System::Clear() {
	Msg("GameDepot::System::Clear()\n");
	m_MountedGames.clear();
}

void GameDepot::System::SetMount(uint32_t param_1, bool param_2) {
	Msg("GameDepot::System::SetMount()\n");
}

void GameDepot::System::MarkGameAsMounted(const std::string param_1) {
	Msg("GameDepot::System::MarkGameAsMounted()\n");
}

const std::list<IGameDepotSystem::Information>& GameDepot::System::GetList() {
	Msg("GameDepot::System::GetList()\n");
	return m_MountedGames;
}

int GameDepot::System::GetRefreshCount()
{
	return m_RefreshCount;
}

GameDepot::System::System() : m_MountedGames() {
	m_RefreshCount = 0;
	Msg("GameDepot_System()\n");
}

void GameDepot::System::FindGame(std::string* param_1) {
	Msg("GameDepot::System::FindGame()\n");
}

void GameDepot::System::MountAsSteampipe(Information* param_1, bool param_2) {
	Msg("GameDepot::System::MountAsSteampipe()\n");
}

void GameDepot::System::Mount(Information* param_1, bool param_2) {
	Msg("GameDepot::System::Mount()\n");
}

void GameDepot::System::MountAsFallback(Information* param_1) {
	Msg("GameDepot::System::MountAsFallback()\n");
}

#define GAMEDEPOTSYSTEM "gamedepotsystem"

void GameDepot::System::Load() {
	KeyValues* kv = new KeyValues(GAMEDEPOTSYSTEM); RunCodeAtScopeExit(kv->deleteThis(););

	for (auto& v : m_MountedGames) {
		v.owned = get->SteamApps()->BIsSubscribedApp(v.appid);
		v.installed = get->SteamApps()->BIsAppInstalled(v.appid);
	}

	if (!g_pFullFileSystem->FileExists("cfg/mountdepots.txt", "DEFAULT_WRITE_PATH")) {
		Save(); 
		return;
	}

	kv->LoadFromFile(g_pFullFileSystem, "cfg/mountdepots.txt", "DEFAULT_WRITE_PATH");

	for (auto& v : m_MountedGames)
		v.mounted = false;                                        

	for (KeyValues* pKv = kv->GetFirstSubKey(); pKv; pKv = pKv->GetNextKey()) {
		for (auto& v : m_MountedGames) {
			if (!strcmp(v.folder.c_str(), pKv->GetName())) {
				if (pKv->GetInt())                      
					v.mounted = true;                             
			}
		}
	}
}

void GameDepot::System::Save() {
	KeyValues* kv = new KeyValues(GAMEDEPOTSYSTEM); RunCodeAtScopeExit(kv->deleteThis(););

	for (auto& v : m_MountedGames) {
		if (v.mounted)                                           
			kv->SetInt(v.folder.c_str(), 1);                     
	}

	kv->SaveToFile(g_pFullFileSystem, "cfg/mountdepots.txt", "DEFAULT_WRITE_PATH");
}

#undef GAMEDEPOTSYSTEM

void GameDepot::System::Setup() {
	Msg("GameDepot::System::Setup()\n");
}