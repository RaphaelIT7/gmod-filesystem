#include "GModLanguage.h"
#include "filesystem.h"
#include "convar.h"
#include "garrysmod/public/IGet.h"
#include "garrysmod/public/ILuaShared.h"
#include "garrysmod/public/ILuaInterface.h"
#include "tier1/utlbuffer.h"
#include "Bootil/Bootil.h"
#include <string>
#include <codecvt>
#include <locale>

static void language_Changed( IConVar* pVar, const char* pOldValue, float fOldValue )
{
	if (!g_pFullFileSystem)
		return;

	ConVarRef pCVar( pVar );
	const char* pszLanguage = pCVar.GetString();
	if ( !pszLanguage || strlen( pszLanguage ) <= 1 )
		return;

	g_pFullFileSystem->Language()->ChangeLanguage( pszLanguage );
}

static ConVar gmod_language( "gmod_language", "", FCVAR_ARCHIVE, "Changes language of Garry's mod", language_Changed );

struct LanguageMap
{
	const char* pszLangCode;
	const char* pszLangEngine;
};

static constexpr LanguageMap g_Languages[] =
{
	{ "en",    "english" },
	{ "da",    "danish" },
	{ "nl",    "dutch" },
	{ "fi",    "finnish" },
	{ "fr",    "french" },
	{ "de",    "german" },
	{ "it",    "italian" },
	{ "ko",    "koreana" },
	{ "no",    "norwegian" },
	{ "pl",    "polish" },
	{ "pt-PT", "portuguese" },
	{ "ru",    "russian" },
	{ "zh-CN", "schinese" },
	{ "zh-TW", "tchinese" },
	{ "es-ES", "spanish" },
	{ "sv-SE", "swedish" },
	{ "th",    "thai" },
	{ "ja",    "japanese" },
	{ "hu",    "hungarian" },
	{ "cs",    "czech" },
	{ "tr",    "turkish" },
	{ "bg",    "bulgarian" },
	{ "el",    "greek" },
};

void CLanguage::ChangeLanguage( const char* pszLangCode, bool bForceReload )
{
	if ( !pszLangCode )
		return;

	if ( !bForceReload && V_strncmp( pszLangCode, m_szLastLangCode, sizeof( m_szLastLangCode ) ) == 0 )
		return;

	strncpy( m_szLastLangCode, pszLangCode, sizeof( m_szLastLangCode ) );
	if ( gmod_language.GetString()[0] == '\0' )
		gmod_language.SetValue( pszLangCode );

	std::vector<std::string> files;
	{
		FileFindHandle_t hFindHandle;
		const char* pszFile = g_pFullFileSystem->FindFirstEx( "resource/localization/en/*.properties", "GAME", &hFindHandle );
		while ( pszFile )
		{
			files.emplace_back( "resource/localization/en/" + std::string( pszFile ) );
			pszFile = g_pFullFileSystem->FindNext( hFindHandle );
		}

		g_pFullFileSystem->FindClose( hFindHandle );
	}


	if ( strcmp( pszLangCode, "en" ) != 0 )
	{
		std::string strLangPath = "resource/localization/";
		strLangPath += pszLangCode;
		strLangPath += "/";

		FileFindHandle_t pFindHandle;
		std::string strWildcard = strLangPath + "*.properties";
		const char* pszFile = g_pFullFileSystem->FindFirstEx( strWildcard.c_str(), "GAME", &pFindHandle );
		while ( pszFile )
		{
			files.emplace_back( strLangPath + pszFile );
			pszFile = g_pFullFileSystem->FindNext( pFindHandle );
		}

		g_pFullFileSystem->FindClose( pFindHandle );
	}

	m_Strings.clear();
	for ( const auto& file : files )
		ProcessFile( file, "GAME" );

	if ( !bForceReload )
		TellLuaLanguageChanged( pszLangCode );
}

void CLanguage::ChangeLanguage_Steam( const char *pszLanguage )
{
	if ( !pszLanguage )
	{
		ChangeLanguage( "en", false );
		return;
	}

	const char* pszID = nullptr;
	for ( const auto& lang : g_Languages )
	{
		if ( !strcmp( gmod_language.GetString(), lang.pszLangEngine ) )
		{
			pszID = lang.pszLangCode;
			break;
		}
	}

	if ( !pszID )
	{
		DevMsg( "[Language] Not Found: %s\n", pszLanguage );
		pszID = "en";
	}

	ChangeLanguage( pszID, false );
}

void CLanguage::UpdateSourceEngineLanguage()
{
	const char* pszEngineLanguage = nullptr;
	for ( const auto& lang : g_Languages )
	{
		if ( !strcmp( gmod_language.GetString(), lang.pszLangCode ) )
		{
			pszEngineLanguage = lang.pszLangEngine;
			break;
		}
	}

	if ( !pszEngineLanguage )
	{
		DevMsg( "[Language] Cannot find Source Engine language for '%s'\n", gmod_language.GetString() );
		pszEngineLanguage = "english";
	}

	ConVarRef clLanguage( "cl_language" );
	if ( clLanguage.IsValid() )
		clLanguage.SetValue( pszEngineLanguage );
}

void CLanguage::TellLuaLanguageChanged( const char *pszLangCode )
{
	if (!get)
		return;

	GarrysMod::Lua::ILuaShared* shared = (GarrysMod::Lua::ILuaShared*)get->LuaShared();
	GarrysMod::Lua::ILuaInterface* LUA = shared->GetLuaInterface( GarrysMod::Lua::State::MENU );
	if ( LUA )
	{
		LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
			LUA->GetField( -1, "LanguageChanged" );
			if ( LUA->IsType(-1, GarrysMod::Lua::Type::Function ) )
			{
				LUA->PushString( pszLangCode );
				LUA->Call( 1, 0 );
			} else {
				LUA->Pop(1);
			}

		LUA->Pop(1);
	}
}

void CLanguage::ReloadLanguage()
{
	// RaphaelIT7: GMod does something else here
	ChangeLanguage( gmod_language.GetString(), true );
}

static inline std::wstring UTF8ToUTF16( const std::string &utf8Str )
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes( utf8Str );
}

bool CLanguage::GetString( const char *pszPhraseKey, wchar_t *pszPhraseOut, uint32_t nPhraseOutLength )
{
	if (!pszPhraseKey)
		return false;

	const char* pszLookupKey = pszPhraseKey;
	auto it = m_Strings.find( pszLookupKey );
	if ( it == m_Strings.end() )
	{
		pszLookupKey = pszPhraseKey + 1;
		it = m_Strings.find( pszLookupKey );

		if ( it == m_Strings.end() )
			return false;
	}

	memset(pszPhraseOut, 0, sizeof(wchar_t) * nPhraseOutLength);

	const std::wstring& value = it->second;
	uint32_t length = static_cast<uint32_t>( value.length() );
	if ( length >= nPhraseOutLength )
		length = nPhraseOutLength - 1;

	memcpy( pszPhraseOut, value.data(), length * sizeof(wchar_t) );
	return true;
}

static inline uint16_t ParseHex4( const std::wstring &str, size_t pos )
{
	uint16_t value = 0;
	for ( int i=0; i<4; ++i )
	{
		wchar_t c = str[pos + i];
		value <<= 4;

		if ( c >= L'0' && c <= L'9' )
			value |= c - L'0';
		else if ( c >= L'a' && c <= L'f' )
			value |= c - L'a' + 10;
		else if ( c >= L'A' && c <= L'F' )
			value |= c - L'A' + 10;
	}

	return value;
}

// RaphaelIT7:
// This looks like shit but I pray it works
// The goal is to mimic GMod's ParseString function
static std::wstring ParseString( const std::string &strInput )
{
	wchar_t unicode[4096];
	size_t unicodeLen = Q_UTF8ToUTF32( strInput.c_str(), (uchar32*)unicode, sizeof(unicode) );

	std::wstring strOutput;
	for ( size_t i=0; i<unicodeLen; ++i )
	{
		if ( unicode[i] != U'\\' )
		{
			strOutput.push_back( unicode[i] );
			++i;
			continue;
		}

		if ( unicode[i + 1] == U'u' )
		{
			strOutput.push_back( ParseHex4( unicode, i + 2 ) );
			i += 6;
			continue;
		}

		if ( unicode[i + 1] == U'n' )
		{
			strOutput.push_back( L'\n' );
			i += 2;
			continue;
		}

		++i;
	}

	return strOutput;
}

// RaphaelIT7: This is an absolute mess, but so it looks in IDA
bool CLanguage::ProcessFile( const std::string& strFileName, const char* pathID )
{
	CUtlBuffer buffer;
	if ( !g_pFullFileSystem->ReadFile( strFileName.c_str(), pathID, buffer ) )
	{
		Warning( "Failed to read language file %s\n", strFileName.c_str() );
		return false;
	}

	if ( !buffer.Base() )
	{
		Warning( "Failed to load language file %s\n", strFileName.c_str() );
		return false;
	}

	std::string strContents(static_cast<const char*>( buffer.Base() ), buffer.TellPut() );
	std::vector<std::string> lines;
	Bootil::String::Util::Split( strContents, "\n", lines );

	bool bContinue = false;
	std::string strCurrentKey;
	for ( size_t i = 0; i < lines.size(); ++i )
	{
		std::string line = lines[i];
		if ( !line.empty() && line[0] == '#' )
			continue;

		if ( Bootil::String::Test::ContainsChar( line, '=' ) )
		{
			std::string key = line;
			std::string value = line;

			Bootil::String::Util::TrimAfter( key, "=", true );
			Bootil::String::Util::TrimBefore( value, "=", false );

			int slashCount = 0;
			for ( int j=int( value.size() ) - 1; j >= 0 && value[j] == '\\'; --j )
				++slashCount;

			bContinue = ( slashCount & 1 ) != 0;
			strCurrentKey = key;

			m_Strings[ strCurrentKey ] = ParseString( value );
			continue;
		}

		if ( !bContinue )
			continue;

		std::string continuation = line;
		Bootil::String::Util::TrimLeft( continuation, "\t " );

		int slashCount = 0;
		for ( int j=int( continuation.size() ) - 1; j >= 0 && continuation[j] == '\\'; --j )
			++slashCount;

		bContinue = ( slashCount & 1 ) != 0;

		m_Strings[ strCurrentKey ] = ParseString( continuation );
	}

	return true;
}