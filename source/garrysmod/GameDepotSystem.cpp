#include "GameDepotSystem.h"
#include <stdlib.h>
#include <basetypes.h>
#include "tier0/dbg.h"
#include "tier1/keyvalues.h"
#include "filesystem.h"
#include <sdk_backports.h>
#include "public/IGet.h"
#include <Bootil/Bootil.h>
#include <Bootil/Types/String.h>
#include "steam/isteamapps.h"

static const std::vector<IGameDepotSystem::Information>& MountableGames()
{
	static const std::vector<IGameDepotSystem::Information> mountableGames =
	{
		{ 220,     "Half-Life 2 & Episodes",            "hl2",               false, false, false, false, true,  true, { "hl2", "episodic", "ep2", "lostcoast" }},
		{ 240,     "Counter-Strike: Source",            "cstrike",           false, false, false, false, true,  true  },
		{ 440,     "Team Fortress 2",                   "tf",                false, false, false, false, true,  false },
		{ 300,     "Day of Defeat: Source",             "dod",               false, false, false, false, true,  false },
		{ 320,     "Half-Life 2: Deathmatch",           "hl2mp",             false, false, false, false, true,  false },
		{ 280,     "Half-Life: Source",                 "hl1",               false, false, false, false, true,  false },
		{ 360,     "Half-Life Deathmatch: Source",      "hl1mp",             false, false, false, false, true,  false },
		{ 22208,   "Zeno Clash (Model Pack)",           "zeno_clash",        false, false, false, false, false, false },
		{ 400,     "Portal",                            "portal",            false, false, false, false, true,  false },
		{ 17530,   "D.I.P.R.I.P.",                      "diprip",            false, false, false, false, false, false },
		{ 17500,   "Zombie Panic! Source",              "zps",               false, false, false, false, false, false },
		{ 17570,   "Pirates, Vikings and Knights II",   "pvkii",             false, false, false, false, false, false },
		{ 17580,   "Dystopia",                          "dystopia",          false, false, false, false, false, false },
		{ 17700,   "Insurgency (Source Mod)",           "insurgency",        false, false, false, false, false, false },
		{ 17510,   "Age of Chivalry",                   "ageofchivalry",     false, false, false, false, false, false },
		{ 550,     "Left 4 Dead 2",                     "left4dead2",        false, false, false, false, true,  false },
		{ 500,     "Left 4 Dead",                       "left4dead",         false, false, false, false, true,  false },
		{ 620,     "Portal 2",                          "portal2",           false, false, false, false, true,  false },
		{ 630,     "Alien Swarm",                       "swarm",             false, false, false, false, true,  false },
		{ 17710,   "Nuclear Dawn",                      "nucleardawn",       false, false, false, false, true,  false },
		{ 70000,   "Dino D-Day",                        "dinodday",          false, false, false, false, true,  false },
		{ 730,     "Counter-Strike: GO (Legacy Branch)","csgo",              false, false, false, false, true,  false },
		{ 225600,  "Blade Symphony",                    "berimbau",          false, false, false, false, true,  false },
		{ 251110,  "INFRA",                             "infra",             false, false, false, false, true,  false },
		{ 265630,  "Fistful of Frags",                  "fof",               false, false, false, false, true,  false },
		{ 221910,  "The Stanley Parable",               "thestanleyparable", false, false, false, false, false, false },
		{ 1224600, "G String Game",                     "gstringv2",         false, false, false, false, true,  false },
		{ 1786950, "Klaus Veen's Treason",              "treason",           false, false, false, false, true,  false },
		{ 1012110, "Military Conflict: Vietnam",        "treason",           false, false, false, false, true,  false },
		{ 362890,  "Black Mesa",                        "bms",               false, false, false, false, true,  false },
	};

	return mountableGames;
}

void GameDepot::System::Refresh()
{
	Msg( "GameDepot::System::Refresh()\n" );
	m_RefreshCount++;
	Clear();
	Load();

	for ( auto &game : m_MountedGames )
	{
		if ( game.owned && game.installed && game.mounted )
			Mount( game, false );
		else if ( game.retail )
			MountAsFallback( game );
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
	Msg( "GameDepot::System::SetMount()\n" );
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
	Msg( "GameDepot::System::MarkGameAsMounted()\n" );
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
	Msg( "GameDepot::System::GetList()\n" );
	return m_MountedGames;
}

int GameDepot::System::GetRefreshCount()
{
	return m_RefreshCount;
}

GameDepot::System::System()
{
	m_RefreshCount = 0;
	Msg( "GameDepot::System::System()\n" );
}

void GameDepot::System::FindGame( std::string &strGameName )
{
	Msg( "GameDepot::System::FindGame()\n" );
}

bool GameDepot::System::MountAsSteampipe( Information &info, bool bHead )
{
#ifdef DEDICATED
	return true;
#else
    std::string installDir = GetAppInstallDir_FixedCase( info.appid );
    if ( installDir.empty() )
        return false;

    for ( const std::string& sub : info.mountSubDirs )
        DoMountDir( info, installDir + "\\" + sub, bHead );

    DoMountDir( info, installDir + "\\", bHead );
    return true;
#endif
}

void GameDepot::System::Mount( Information &info, bool bMount )
{
	DevMsg( "Mounting game '%s' (%s, %i)...\n",
		info.title.c_str(), info.folder.c_str(), info.appid );

	if ( MountAsSteampipe( info, bMount ) )  
		return;

	if ( info.folder == "hl2" )
		return;

	if ( bMount )
	{
		g_pFullFileSystem->AddSearchPath( info.folder.c_str(), "GAME", PRIORITY_GROUP_TAIL( GN_CURRENTGAME ) );
		g_pFullFileSystem->AddSearchPath( info.folder.c_str(), info.folder.c_str(), PRIORITY_GROUP_HEAD( GN_CURRENTGAME ) );
	}
	else
	{
		g_pFullFileSystem->AddSearchPath( info.folder.c_str(), "GAME", PRIORITY_GROUP_TAIL( GN_GAMECONTENT ) );
		g_pFullFileSystem->AddSearchPath( info.folder.c_str(), info.folder.c_str(), PRIORITY_GROUP_HEAD( GN_GAMECONTENT ) );
	}

	g_pFullFileSystem->MarkPathIDByRequestOnly( info.folder.c_str(), true );

	if ( info.enabled )
		g_pFullFileSystem->AddSearchPath("pak01", info.folder.c_str(), 21);
}

void GameDepot::System::MountAsFallback( Information &info )
{
	DevMsg( "Mounting game '%s' as fallback (%s, %i)...\n", info.title.c_str(), info.folder.c_str(), info.appid );

	std::string path = get->GameDir();
	path += "\\sourceengine\\";
	path += "content_";
	path += info.folder.c_str();
	path += ".vpk";

	g_pFullFileSystem->AddSearchPath( path.c_str(), "GAME", PRIORITY_GROUP_TAIL( GN_GAMECONTENT ) );
	g_pFullFileSystem->AddSearchPath( path.c_str(), info.folder.c_str(), PRIORITY_GROUP_TAIL( GN_GAMECONTENT ) );
	g_pFullFileSystem->MarkPathIDByRequestOnly( info.folder.c_str(), true );
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
	Msg( "GameDepot::System::Save()\n" );
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
		// RaphaelIT7: Apparently I was wrong with this
#if 0
		if ( game.owned && game.installed )
		{
			// RaphaelIT7: GetAppInstallDir_FixedCase differs in some way- idk what it is yet.
			std::string strGamePath = GetAppInstallDir_FixedCase( game.appid );
			if ( strGamePath.empty() )
				continue;

			game.folder = strGamePath;
		}
#endif
	}
#endif

	Load();
}

#ifndef DEDICATED
const char* GameDepot::DoMountDir( IGameDepotSystem::Information& info, const std::string& dir, bool bToHead )
{
	const unsigned int addType = bToHead ? PRIORITY_GROUP_HEAD( GN_GAMECONTENT ) : PRIORITY_GROUP_TAIL( GN_GAMECONTENT );
	std::string findPattern = dir + "/*.vpk";

	// TODO: developer convar test I think
	Msg( "MountableGame looking in '%s'...\n", findPattern.c_str() );

	int findHandle = 0;
	const char* pFileName = g_pFullFileSystem->FindFirst( findPattern.c_str(), &findHandle );

	while (pFileName)
	{
		if (pFileName[0] == '.')
		{
			pFileName = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}

		if (!Bootil::String::Test::EndsWith( std::string( pFileName ), "_000.vpk" ) )
		{
			pFileName = g_pFullFileSystem->FindNext( findHandle );
			continue;
		}

		std::string vpkPath = dir + "/" + pFileName;
		Bootil::String::Util::GetFindAndReplace( vpkPath, "_000.vpk", "" );

		// TODO: developer convar test I think
		Msg( "MountableGame adding '%s'\n", vpkPath.c_str() );

		g_pFullFileSystem->AddVPKFileFromPath( vpkPath.c_str(), "GAME", addType );
		g_pFullFileSystem->AddVPKFileFromPath( vpkPath.c_str(), info.folder.c_str(), addType );

		pFileName = g_pFullFileSystem->FindNext( findHandle );
	}

	g_pFullFileSystem->FindClose(findHandle);

	// TODO: developer convar test I think
	Msg("MountableGame adding path '%s'\n", dir.c_str());

	g_pFullFileSystem->AddSearchPath( dir.c_str(), "GAME", addType );
	g_pFullFileSystem->AddSearchPath( dir.c_str(), info.folder.c_str(), addType );
	g_pFullFileSystem->MarkPathIDByRequestOnly( info.folder.c_str(), true );

	return NULL;
}

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