#include "GameDepotSystem.h"
#include <stdlib.h>
#include <basetypes.h>
#include "tier0/dbg.h"
#include "tier1/keyvalues.h"
#include "filesystem.h"
#include <sdk_backports.h>
#include "public/IGet.h"
#include "steam/isteamapps.h"

static const std::vector<IGameDepotSystem::Information>& MountableGames()
{
	static const std::vector<IGameDepotSystem::Information> mountableGames =
	{
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

	return mountableGames;
}

void GameDepot::System::Refresh()
{
	m_RefreshCount++;
	Clear();
	Load();

	for ( auto &v : m_MountedGames )
	{
		if ( v.owned && v.installed && v.mounted )
			Mount( &v, false);
		else if ( v.retail )
			MountAsFallback(&v);
	}
}

void GameDepot::System::Clear()
{
	Msg( "GameDepot::System::Clear()\n" );
	Setup();

	g_pFullFileSystem->RemoveSearchPathsByGroup( PRIORITY_GROUP_HEAD( GN_CURRENTGAME ) );
	g_pFullFileSystem->RemoveSearchPathsByGroup( PRIORITY_GROUP_HEAD( GN_GAMECONTENT ) );
}

void GameDepot::System::SetMount( uint32_t nAppID, bool bMounted )
{
	for ( auto &v : m_MountedGames )
	{
		if (v.appid != nAppID)
			continue;

		if (v.mounted != bMounted)
			m_RefreshCount++;

		v.mounted = bMounted;
		break;
	}
}

void GameDepot::System::MarkGameAsMounted( const std::string strGameFolder )
{
	for ( auto& v : m_MountedGames )
	{
		if ( v.folder == strGameFolder )
		{
			v.mounted = true;
			break;
		}
	}
}

const std::list<IGameDepotSystem::Information> &GameDepot::System::GetList()
{
	return m_MountedGames;
}

int GameDepot::System::GetRefreshCount()
{
	return m_RefreshCount;
}

GameDepot::System::System()
{
	m_RefreshCount = 0;
	Msg( "GameDepot_System()\n" );
}

void GameDepot::System::FindGame( std::string* param_1 )
{
	Msg( "GameDepot::System::FindGame()\n" );
}

bool GameDepot::System::MountAsSteampipe( Information* param_1, bool param_2 )
{
	Msg( "GameDepot::System::MountAsSteampipe()\n" );
	return true; // TODO
	// ^^ I think this is the process of mounting VPK's
	// RaphaelIT7: On Linux DS this does nothing but you can find the code in filesystem_stdio.dylib (macos build)
}

void GameDepot::System::Mount( Information *mountGameInfo, bool mount )
{
	DevMsg( "Mounting game '%s' (%s, %i)...\n",
		mountGameInfo->title.c_str(), mountGameInfo->folder.c_str(), mountGameInfo->appid );

	if ( MountAsSteampipe( mountGameInfo, mount) )  
		return;

	if (mountGameInfo->folder == "hl2")
		return;

	if (mount)
	{
		g_pFullFileSystem->AddSearchPath( mountGameInfo->folder.c_str(), "GAME", PRIORITY_GROUP_TAIL( GN_CURRENTGAME ) );
		g_pFullFileSystem->AddSearchPath( mountGameInfo->folder.c_str(), mountGameInfo->folder.c_str(), PRIORITY_GROUP_HEAD( GN_CURRENTGAME ) );
	}
	else
	{
		g_pFullFileSystem->AddSearchPath( mountGameInfo->folder.c_str(), "GAME", PRIORITY_GROUP_TAIL( GN_GAMECONTENT ) );
		g_pFullFileSystem->AddSearchPath( mountGameInfo->folder.c_str(), mountGameInfo->folder.c_str(), PRIORITY_GROUP_HEAD( GN_GAMECONTENT ) );
	}

	g_pFullFileSystem->MarkPathIDByRequestOnly(mountGameInfo->folder.c_str(), true);

	if (mountGameInfo->enabled)
		g_pFullFileSystem->AddSearchPath("pak01", mountGameInfo->folder.c_str(), 21);
}

void GameDepot::System::MountAsFallback( Information *info )
{
	DevMsg("Mounting game '%s' as fallback (%s, %i)...\n", info->title.c_str(), info->folder.c_str(), info->appid);

	std::string path = get->GameDir();
	path += "\\sourceengine\\";
	path += "content_";
	path += info->title.c_str();
	path += ".vpk";

	g_pFullFileSystem->AddSearchPath( path.c_str(), "GAME", PRIORITY_GROUP_TAIL( GN_GAMECONTENT ) );
	g_pFullFileSystem->AddSearchPath( path.c_str(), info->folder.c_str(), PRIORITY_GROUP_TAIL( GN_GAMECONTENT ) );
	g_pFullFileSystem->MarkPathIDByRequestOnly( info->folder.c_str(), true );
}

#define GAMEDEPOTSYSTEM "gamedepotsystem"
void GameDepot::System::Load()
{
	KeyValues* kv = new KeyValues( GAMEDEPOTSYSTEM );
	RunCodeAtScopeExit( kv->deleteThis(); );

	if ( !g_pFullFileSystem->FileExists( "cfg/mountdepots.txt", "DEFAULT_WRITE_PATH" ) )
	{
		Save();
		return;
	}

	kv->LoadFromFile( g_pFullFileSystem, "cfg/mountdepots.txt", "DEFAULT_WRITE_PATH" );

	for ( auto& v : m_MountedGames )
		v.mounted = false;

	for ( KeyValues* pKv = kv->GetFirstSubKey(); pKv; pKv = pKv->GetNextKey() )
	{
		for ( auto& v : m_MountedGames)
		{
			if ( !strcmp( v.folder.c_str(), pKv->GetName() ) )
			{
				if ( pKv->GetInt() )
					v.mounted = true;
			}
		}
	}
}

void GameDepot::System::Save()
{
	KeyValues* kv = new KeyValues( GAMEDEPOTSYSTEM );
	RunCodeAtScopeExit( kv->deleteThis(); );

	for ( auto& v : m_MountedGames )
	{
		if ( v.mounted )
			kv->SetInt( v.folder.c_str(), 1 );
	}

	kv->SaveToFile( g_pFullFileSystem, "cfg/mountdepots.txt", "DEFAULT_WRITE_PATH" );
}

#undef GAMEDEPOTSYSTEM

static void FillDepotList( std::list<IGameDepotSystem::Information> &MountedGames )
{
	MountedGames.clear();
	const std::vector<IGameDepotSystem::Information> &mountableGames = MountableGames();
	MountedGames.resize( mountableGames.size() );

	for ( size_t i = 0; i < mountableGames.size(); ++i ) 
		MountedGames.push_back( mountableGames[i] );
}

bool GameDepot::bInitialized = false;
void GameDepot::System::Setup()
{
	if ( GameDepot::bInitialized )
		return;

	GameDepot::bInitialized = true;
	FillDepotList( m_MountedGames );

#ifdef DEDICATED
	for ( auto& game : m_MountedGames )
	{
		game.owned = true;
		game.installed = true;
	}
#else
	for ( auto& game : m_MountedGames )
	{
		game.owned = SteamApps()->BIsSubscribedApp( game.appid );
		game.installed = SteamApps()->BIsAppInstalled( game.appid );
		if ( game.owned && game.installed )
		{
			// RaphaelIT7: GetAppInstallDir_FixedCase differs in some way- idk what it is yet.
			std::string strGamePath = GetAppInstallDir_FixedCase( game.appid );
			if ( strGamePath.empty() )
				continue;

			game.folder = strGamePath;
		}
	}
#endif

	Load();
}

#ifndef DEDICATED
std::string GameDepot::GetAppInstallDir_FixedCase( int nAppID )
{
	static std::string empty = ""; // Just in case to avoid issues.

	char szPath[260];
	uint32_t len = SteamApps()->GetAppInstallDir( nAppID, szPath, sizeof( szPath ) );
	if ( !len )
		return empty;

	return szPath;
}
#endif