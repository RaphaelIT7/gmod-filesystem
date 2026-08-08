#pragma once

#include "public/IGameDepotSystem.h"
#include "GameDepotSystem.h"
#include <stdlib.h>
#include <basetypes.h>
#include "tier0/dbg.h"

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

void GameDepot_System::Refresh() {
	Msg("GameDepot_System::Refresh()\n");
}

void GameDepot_System::Clear() {
	Msg("GameDepot_System::Clear()\n");
}

void GameDepot_System::Save() {
	Msg("GameDepot_System::Save()\n");
}

void GameDepot_System::SetMount(uint32_t param_1, bool param_2) {
	Msg("GameDepot_System::SetMount()\n");
}

void GameDepot_System::MarkGameAsMounted(const std::string param_1) {
	Msg("GameDepot_System::MarkGameAsMounted()\n");
}

const std::list<IGameDepotSystem::Information>& GameDepot_System::GetList() const {
	Msg("GameDepot_System::GetList()\n");
	static const std::list<IGameDepotSystem::Information> empty;
	return empty;
}

void GameDepot_System::MountAsMapFix(uint32_t param_1) {
	Msg("GameDepot_System::MountAsMapFix()\n");
}

void GameDepot_System::MountCurrentGame(const std::string& param_1) {
	Msg("GameDepot_System::MountCurrentGame()\n");
}

GameDepot_System::GameDepot_System() {
	Msg("GameDepot_System()\n");
}

void GameDepot_System::FindGame(std::string* param_1) {
	Msg("GameDepot_System::FindGame()\n");
}

void GameDepot_System::MountAsSteampipe(Information* param_1, bool param_2) {
	Msg("GameDepot_System::MountAsSteampipe()\n");
}

void GameDepot_System::Mount(Information* param_1, bool param_2) {
	Msg("GameDepot_System::Mount()\n");
}

void GameDepot_System::MountAsFallback(Information* param_1) {
	Msg("GameDepot_System::MountAsFallback()\n");
}

void GameDepot_System::Load() {
	Msg("GameDepot_System::Load()\n");
}

void GameDepot_System::Setup() {
	Msg("GameDepot_System::Setup()\n");
}