#include "gmad/AddonWhiteList.h"

std::string Addon::WhiteList::FilenameErrors( const std::string &strFileName, uint64_t nFileSize )
{
    for ( unsigned char c : strFileName )
    {
        if ( c <= 31 )
            return "Filename contains invalid character: Code point " + std::to_string( c );

        static constexpr const char *badChars = "\"*:<>?|";
        if ( strchr( badChars, c ) )
            return "Filename contains invalid character '" + std::string( 1, c ) + "': Code point " + std::to_string( c );
    }

    for ( int i = 0; WhiteList::Wildcard[i] != nullptr; ++i )
    {
        const char *pszPattern = WhiteList::Wildcard[i];
        bool bMatched = Bootil::String::Test::Wildcard( pszPattern, strFileName );
        if ( bMatched && ( strcmp( pszPattern, "gamemodes/*/*.txt" ) == 0 || strcmp( pszPattern, "gamemodes/*/*.fgd" ) == 0 ) )
        {
            int slashCount = 0;
            for ( char c : strFileName )
            {
                if ( c == '/' )
                    ++slashCount;
            }

            if ( slashCount >= 3 )
                bMatched = false;
        }

        if ( !bMatched )
            continue;

        if ( strcmp( pszPattern, "gamemodes/*/content/data/*.txt" ) == 0 ||
             strcmp( pszPattern, "gamemodes/*/content/maps/*_ttt.txt" ) == 0 )
        {
            if ( nFileSize > 0x65B97160ULL )
                continue;
        }

        return "";
    }

    return "File is not allowed by whitelist";
}