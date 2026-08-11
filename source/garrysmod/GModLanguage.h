#pragma once

#include "garrysmod/public/IGModLanguage.h"
#include "unordered_stuff.h"

class CLanguage : public IGModLanguage
{
public: // ILanguage
	void ChangeLanguage( const char *pszLangCode, bool bForceReload = false ) override;
	void ChangeLanguage_Steam( const char *pszLanguage ) override;
	void ReloadLanguage() override;
	bool GetString( const char *pszPhraseKey, wchar_t *pszPhraseOut, uint32_t nPhraseOutLength ) override;
	void UpdateSourceEngineLanguage() override;

public: // CLanguage
	void TellLuaLanguageChanged( const char *pszLangCode );
	bool ProcessFile( const std::string& filename, const char* pathID );
	std::wstring ParseString( const std::wstring &strInput );

private:
	char m_szLastLangCode[32]{};
	unordered_map<std::string, std::wstring, StringHash, StringEq> m_Strings;
};