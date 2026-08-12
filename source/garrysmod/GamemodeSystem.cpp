#include "GamemodeSystem.h"
#include "filesystem.h"
#include "public/IGet.h"
#include "garrysmod/public/IMenuSystem.h"
#include "keyvalues.h"
#include <sdk_backports.h>

static void gamemode_Changed( IConVar *pVar, const char *pOldValue, float flOldValue )
{
	if (!g_pFullFileSystem)
		return;

	ConVarRef pCVar( pVar );
	g_pFullFileSystem->Gamemodes()->SetActive( pCVar.GetString() );
	g_pFullFileSystem->DoFilesystemRefresh();
}
static ConVar gamemode( "gamemode", "sandbox", 0, "The current gamemode", gamemode_Changed );

void Gamemode::System::OnJoinServer( const std::string& strGamemode )
{
	if ( ChangeGamemode( strGamemode, false ) )
		g_pFullFileSystem->DoFilesystemRefresh();
}

void Gamemode::System::OnLeaveServer()
{
	if ( ChangeGamemode( gamemode.GetString(), false ) )
		g_pFullFileSystem->DoFilesystemRefresh();
}

uint64_t StringToUInt64( const char* str )
{
	char* end;
	return strtoull( str, &end, 10 );
}

void Gamemode::System::Refresh()
{
	Clear();

	FileFindHandle_t hFindHandle;
	const char* pszFileName = g_pFullFileSystem->FindFirstEx( "gamemodes/*", "GAME", &hFindHandle );
	while ( pszFileName )
	{
		if ( g_pFullFileSystem->FindIsDirectory( hFindHandle ) )
			AddGamemode( pszFileName );

		pszFileName = g_pFullFileSystem->FindNext( hFindHandle );
	}

	g_pFullFileSystem->FindClose( hFindHandle );

	SetActive( (std::string)gamemode.GetString() );
}

void Gamemode::System::Clear()
{
	m_Gamemodes.clear();
	g_pFullFileSystem->RemoveSearchPathsByGroup( PRIORITY_GROUP_HEAD( GN_GMCONTENT ) );

	// RaphaelIT7: Funny issue- if we unregister with -1 for whatever reason it nukes all our ConVars
	if ( m_ConVarIdentifier == -1 )
		return;

	// RaphaelIT7:
	// Our custom setup to not leak memory- "plans"
	// The issue is that UnregisterConCommands destroys the entire chain...
	// We added ConVar::SetLocalDLLIdentifier so that we can easily unregister all of them at once like below
	// Doing many one by one UnregisterConCommand calls would waste performance.
	g_pCVar->UnregisterConCommands( m_ConVarIdentifier );

	// RaphaelIT7 - Update:
	// Learned the hard way why this probably isn't done.
	// It will crash in vstdlib for whatever reason even though its not expected to keep any references anymore.
	// ToDo:
	// So doing the above code for s_nDLLIdentifier fixed the vstdlib crash
	// UnregisterConCommands will already fix up the m_pNext chain
	// Right now were getting else a crash in the menusystem.dll and I'm just gonna guess that it's trying to iterate convars.
	// m_ConVarArena.release();
	// m_ConVars.clear();
}

const IGamemodeSystem::Information& Gamemode::System::Active()
{
	return FindByName( m_strActive );
}

const IGamemodeSystem::Information &Gamemode::System::FindByName( const std::string& strGamemode )
{
	for ( const IGamemodeSystem::Information& info : m_Gamemodes )
	{
		if ( info.name == strGamemode )
			return info;
	}

	m_NotFound = IGamemodeSystem::Information();
	m_NotFound.exists = false;
	m_NotFound.name = strGamemode;

	return m_NotFound;
}

void Gamemode::System::SetActive( const std::string& strGamemode )
{
	for ( IGamemodeSystem::Information info : m_Gamemodes )
	{
		if ( info.name == strGamemode )
		{
			m_strActive = strGamemode;
			break;
		}
	}
}

const std::list<IGamemodeSystem::Information>& Gamemode::System::GetList() const
{
	return m_Gamemodes;
}

bool Gamemode::System::IsServerBlacklisted( char const* address, char const* hostname, char const* description, char const* gm, char const* map )
{
	return get->MenuSystem()->IsServerBlacklisted( address, hostname, description, gm, map );
}

// RaphaelIT7:
// Just spent like an hour to figure out why it was crashing.
// This one is sneakily called from CServerAddons::MountDownloadedAddons (client.dll - "WorkshopDL: Failed to mount %s\n") as the very last thing
void Gamemode::System::OnServerDownloadsMounted()
{
	// Do some fancy checks? Idk
	Refresh();
}

Gamemode::System::System()
{
	m_strActive = "sandbox";
}

bool Gamemode::System::ChangeGamemode( const std::string& strGamemode, bool bRestore )
{
	if ( strGamemode == m_strActive )
		return false;

	IGamemodeSystem::Information info = FindByName( strGamemode );
	if ( !info.exists )
		return false;

	Msg( "%s gamemode to %s\n", bRestore ? "Restoring" : "Switching", strGamemode.c_str() );
	SetActive( strGamemode );
	Refresh();
	return true;
}

bool IsValidConsoleName( const char* s )
{
	for ( const char* p=s; *p; ++p )
	{
		const unsigned char c = *p;
		if (
			(c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') ||
			c == '+' || c == '-' ||
			c == '.' || c == '_' ||
			c == '^' || c == '`' ||
			c == '!' || c == '~'
			)
		{
			continue;
		}

		return false;
	}

	return true;
}

void Gamemode::System::AddGamemode( std::string strGamemode )
{
	// Gmod adds the gamemode inside Gamemode::System::AddGamemode
	KeyValues* kv = new KeyValues( strGamemode.c_str() );
	RunCodeAtScopeExit( kv->deleteThis(); );

	std::string strPath = "gamemodes/" + strGamemode + "/" + strGamemode + ".txt";
	if ( !kv->LoadFromFile( g_pFullFileSystem, strPath.c_str(), "GAME" ) )
		return;

	// RaphaelIT7: We need to have m_ConVarIdentifier set at this point as else convar registration will be screwed
	if ( m_ConVarIdentifier == -1 )
		m_ConVarIdentifier = g_pCVar->AllocateDLLIdentifier();

	IGamemodeSystem::Information information;
	KeyValues* settings = kv->FindKey("settings");
	if (settings)
	{
		for ( KeyValues *sub = settings->GetFirstSubKey(); sub; sub = sub->GetNextKey() )
		{
			if (sub->GetBool("dontcreate", false))
				continue;

			const char* name = sub->GetString( "name", nullptr );
			if ( !name )
			{
				Warning( "Gamemode '%s' has a convar in 'settings' block without a 'name' key!\n", strGamemode.c_str() );
				continue;
			}

			if ( !IsValidConsoleName( name ) )
			{
				Warning("Not registering convar '%s' for gamemode '%s' beacuse it has invalid symbols!\n", name, strGamemode.c_str() );
				continue;
			}

			if ( strlen( name ) <= 1 )
			{
				Warning( "Not registering convar '%s' for gamemode '%s' beacuse its name is too short!\n", name, strGamemode.c_str() );
				continue;
			}

			// RaphaelIT7:
			// This one I reported a long while ago xD
			// BUT GMod checks this differently... Instead of one search they do two?
			// One for FindVar & one FindCommand
			const ConCommandBase* pVar = g_pCVar->FindCommandBase( name );
			if ( pVar && pVar->IsCommand() )
			{
				Warning( "Not registering convar '%s' for gamemode '%s' because there's a command with that name!\n", name, strGamemode.c_str() );
				continue;
			}

			const char* text = sub->GetString( "text", "" );
			const char* help = sub->GetString( "help", "" );
			const char* def = sub->GetString( "default", "" );
			int flags = ( sub->GetBool( "replicate", true ) == 0 ) ? ( FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_LUA_SERVER ) : ( FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_LUA_SERVER );

			// RaphaelIT7:
			// GMod issue! GMod leaks memory!
			// We can do better :)
			int* DLLIdentifier = ConVar_GetDLLIdentifier();
			int CurrentID = *DLLIdentifier;
			// This probably isn't even needed for adding and probably only for removing
			*DLLIdentifier = m_ConVarIdentifier; // So that the convar is registered as a m_ConVarIdentifier!

			ConVar* pConVar = ::new ( AllocConVar() ) ConVar( AllocString( name ), AllocString( def ), flags, AllocString( help ));
			pConVar->SetLocalDLLIdentifier( m_ConVarIdentifier );

			*DLLIdentifier = CurrentID;
		}
	}
			
	information.exists = true;
	information.title = kv->GetString( "title", "Missing Title" );
	information.name = kv->GetString( "name", strGamemode.c_str() );
	information.basename = kv->GetString( "base", "base" );
	information.category = kv->GetString( "category", "" );
	information.maps = kv->GetString( "maps", "");
	information.menusystem = kv->GetBool( "menusystem", false );
	information.workshopid = StringToUInt64( kv->GetString( "workshopid", "0" ) );

	if (information.name == "")
		information.name = information.title;

	m_Gamemodes.push_back( information );
}
