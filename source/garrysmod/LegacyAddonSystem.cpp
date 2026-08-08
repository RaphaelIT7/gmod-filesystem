#include "LegacyAddonSystem.h"
#include "filesystem.h"

void LegacyAddons::System::Refresh()
{
	m_pAddons.clear();
	g_pFullFileSystem->RemoveSearchPathsByGroup( PRIORITY_GROUP_HEAD( GN_ADDONCONTENT ) );

	FileFindHandle_t findHandle;
	const char* pszFileName = g_pFullFileSystem->FindFirstEx("addons/*", nullptr, &findHandle );
	while ( pszFileName )
	{
		if( g_pFullFileSystem->FindIsDirectory( findHandle ) )
		{
			std::string strPath = "addons/";
			strPath = strPath + pszFileName;

			char szFullPath[1024];
			g_pFullFileSystem->RelativePathToFullPath( strPath.c_str(), "MOD", szFullPath, sizeof(szFullPath) );

			g_pFullFileSystem->AddSearchPath(szFullPath, "GAME");
			g_pFullFileSystem->AddSearchPath(szFullPath, "thirdparty");

			ILegacyAddons::Information information;
			information.name = pszFileName;
			information.path = (std::string)szFullPath;
			information.luapath = strPath;
			information.placeholder4 = ""; // ToDo: Find out.

			m_pAddons.push_back(information);
		}

		pszFileName = g_pFullFileSystem->FindNext( findHandle );
	}
}

const std::list<ILegacyAddons::Information>& LegacyAddons::System::GetList() const
{
	return m_pAddons;
}