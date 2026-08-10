#pragma once

#include "filesystem.h"
#include "Bootil/Bootil.h"
#include "garrysmod/gmad/AddonFormat.h"

// RaphaelIT7:
// The Addon::Reader from GMad is not at all the same as the one in GMod
// IMPORTANT:
// The Addon::Reader below is not thread safe!
// Specifically the OpenFile function as it uses a static buffer!

namespace Addon
{

class Reader
{
public:
	Reader( uint64_t wsid );
	bool ShouldBeExtracted( const std::string &strFileName );
	bool OpenFile( const std::string &strFileName );
	bool ReadAddonFile( FileHandle_t hFileHandle, Bootil::Buffer &buffer );
	// RaphaelIT7: GMod also seems to pass the hFileHandle yet it's completely unused
	bool ReadRequiredContent( Bootil::Buffer &buffer );
	// RaphaelIT7: Same thing as above.
	bool ReadFileEntry( Bootil::Buffer &buffer );
	void ExtractFiles();
	bool BuildOffsets();
	FileHandle_t GetPackFile();
	uint32_t GetNumFiles();
	Addon::FileEntry &GetFile( uint32_t nFileID );

private:
	std::string m_strFileName;
	uint32_t m_nAddonSize = 0;
	char m_nGMAVersion = 0;
	bool m_bBadAddon = false;
	uint64_t m_nWSID = 0;
	// RaphaelIT7: Unused by GMad - usually always 0
	uint64_t m_nSteamID = 0;
	uint64_t m_nTimestamp = 0;
	uint64_t m_nContentsOffset = 0;
	std::string m_strAddonTitle;
	std::string m_strAddonDesc;
	std::string m_strAddonAuthor;
	// RaphaelIT7: Should always be 1 as GMad never writes anything else
	// (also GMad uses a int32_t not uint32_t)
	int32_t m_nAddonVersion = 0;
	// RaphaelIT7: GMod loves to use std::list, wouldn't a std::vector be better here?
	std::list<Addon::FileEntry> m_Files;
#if 0
	// RaphaelIT7: unused in GMod
	std::list<std::string> m_RequiredContent;
#endif
};

}