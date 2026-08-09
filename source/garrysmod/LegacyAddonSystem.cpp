#include "LegacyAddonSystem.h"
#include "filesystem.h"

void LegacyAddons::System::Refresh()
{
	m_pAddons.clear();
	g_pFullFileSystem->RemoveSearchPathsByGroup( PRIORITY_GROUP_HEAD( GN_ADDONCONTENT ) );

	// RaphaelIT7: GMod Bug! GMod never removes GN_BADDONCONTENT
	g_pFullFileSystem->RemoveSearchPathsByGroup( PRIORITY_GROUP_HEAD( GN_BADDONCONTENT ) );

	FileFindHandle_t hFindHandle;
	// RaphaelIT7: Yes GMod passes NULL, not "MOD" but we give it MOD as else we include stuff we DONT want!
	const char* pszFileName = g_pFullFileSystem->FindFirstEx( "addons/*", "MOD", &hFindHandle );
	while ( pszFileName )
	{
		// RaphaelIT7: It should NOT start with .
		if( g_pFullFileSystem->FindIsDirectory( hFindHandle ) && *pszFileName != '.' )
		{
			int priority = PRIORITY_GROUP_TAIL( GN_ADDONCONTENT );
			std::string strPath = "addons/";
			strPath = strPath + pszFileName;

			Msg( "Adding Filesystem Addon '%s'\n", strPath.c_str() );

			char szFullPath[MAX_PATH];
			g_pFullFileSystem->RelativePathToFullPath( strPath.c_str(), "MOD", szFullPath, sizeof(szFullPath) );

			std::string strFullPath = szFullPath;

			bool containsGameScripts = g_pFullFileSystem->FileExists((strFullPath + "/scripts/game_sounds_manifest.txt").c_str()) &&
				g_pFullFileSystem->FileExists((strFullPath + "/scripts/game_sounds_physics.txt").c_str()) &&
				g_pFullFileSystem->FileExists((strFullPath + "/scripts/propdata.txt").c_str());

			if (containsGameScripts)
			{
				Warning("\tAddon contain game scripts?! Lowering mount priority.\n");
				priority = PRIORITY_GROUP_HEAD( GN_BADDONCONTENT );
			}

			bool containsGameScripts2 = g_pFullFileSystem->FileExists((strFullPath + "/scripts/kb_act.lst").c_str()) &&
				g_pFullFileSystem->FileExists((strFullPath + "/scripts/kb_def.lst").c_str()) &&
				g_pFullFileSystem->FileExists((strFullPath + "/scripts/soundmixers.txt").c_str());

			if (containsGameScripts2)
			{
				Warning("\tAddon contain game scripts?! Lowering mount priority. (2)\n");
				priority = PRIORITY_GROUP_HEAD( GN_BADDONCONTENT );
			}

			bool containsGameResources = g_pFullFileSystem->FileExists((strFullPath + "/resource/loadingdialog.res").c_str()) &&
				g_pFullFileSystem->FileExists((strFullPath + "/resource/clientscheme.res").c_str()) &&
				g_pFullFileSystem->FileExists((strFullPath + "/resource/gamemenu.res").c_str());

			if (containsGameResources)
			{
				Warning("\tAddon contains game resources?! Lowering mount priority.\n");
				priority = PRIORITY_GROUP_TAIL( GN_BADDONCONTENT );
			}

			g_pFullFileSystem->AddSearchPath( szFullPath, "GAME", priority );
			g_pFullFileSystem->AddSearchPath( szFullPath, "thirdparty", priority );

			ILegacyAddons::Information information;
			information.name = pszFileName;
			information.path = (std::string)szFullPath;
			information.luapath = strPath + "/lua";
			information.gamemodepath = strPath + "/gamemodes";

			if ( !g_pFullFileSystem->IsDirectory( information.luapath.c_str() ) )
				information.luapath = "";

			if ( !g_pFullFileSystem->IsDirectory( information.gamemodepath.c_str() ) )
				information.gamemodepath = "";

			m_pAddons.push_back( information );
		}

		pszFileName = g_pFullFileSystem->FindNext( hFindHandle );
	}

	g_pFullFileSystem->FindClose( hFindHandle );
}

const std::list<ILegacyAddons::Information>& LegacyAddons::System::GetList() const
{
	return m_pAddons;
}