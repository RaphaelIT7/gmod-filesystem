#include "Tasks.h"

void Addon::Task::ClearUnusedGMAs::Start()
{
	Msg( "ClearUnusedGMAs: Starting work...\n" );
}

void Addon::Task::ClearUnusedGMAs::Cycle()
{
    FileFindHandle_t hFindHandle;
    const char *pszFile = g_pFullFileSystem->FindFirstEx( "cache/workshop/*.gma", "MOD", &hFindHandle );
    while ( pszFile )
    {
        std::string workshopID = pszFile;
        Bootil::String::File::StripExtension( workshopID );
        uint64 wsID = Bootil::String::To::UInt64( workshopID );
        if ( wsID == 0 )
        {
            Warning( "Removing '%s' - Invalid filename, must be Workshop ID\n", pszFile );

            std::string strFilePath = "cache/workshop/";
            strFilePath.append( pszFile );
            g_pFullFileSystem->RemoveFile( strFilePath.c_str(), "MOD" );
        }
        else
        {
            uint32 itemState = SteamUGC()->GetItemState( wsID );
            if ( !( itemState & k_EItemStateInstalled ) )
            {
                Warning( "Removing '%s' - Workshop item is no longer installed\n", pszFile );
                std::string strFilePath = "cache/workshop/";
                strFilePath.append( pszFile );
                g_pFullFileSystem->RemoveFile( strFilePath.c_str(), "MOD" );
            }
        }

        pszFile = g_pFullFileSystem->FindNext( hFindHandle );
    }
    g_pFullFileSystem->FindClose( hFindHandle );

    Msg( "ClearUnusedGMAs: Finished!\n" );
    m_bIsFinished = true;
}

// Weird, I can't find one in IDA...
bool Addon::Task::ClearUnusedGMAs::Finished()
{
	return m_bIsFinished;
}