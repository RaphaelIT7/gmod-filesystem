#include "AddonReader.h"
#include "gmad/AddonWhiteList.h"
#include "sdk_backports.h"
#include "tier1/utlbuffer.h"

Addon::Reader::Reader( uint64_t wsid )
{
	m_nWSID = wsid;
}

// RaphaelIT7:
// With Extracted it means that they quite literally will be written onto disk.
// So you better be very sure that they are safe.
bool Addon::Reader::ShouldBeExtracted( const std::string &strFileName )
{
	if ( Bootil::String::Test::Wildcard( "resource/fonts/*.ttf", strFileName ) )
		return true;

	if ( Bootil::String::Test::Wildcard( "gamemodes/*/content/resource/fonts/*.ttf", strFileName ) )
		return true;

	return false;
}

bool Addon::Reader::OpenFile( const std::string &strFileName )
{
	m_strFileName = strFileName;
	FileHandle_t hFileHandle = g_pFullFileSystem->Open( strFileName.c_str(), "rb", "MOD" );
	// RaphaelIT7 (ToDo):
	// Why does GMod do this?
	// Figure it out as opening GAME is expensive!
	if ( !hFileHandle )
		hFileHandle = g_pFullFileSystem->Open( strFileName.c_str(), "rb", "GAME" );

	if ( !hFileHandle )
	{
		Warning( "Not loading addon '%s' - file doesn't exist\n", m_strFileName.c_str() );
		return false;
	}

	RunCodeAtScopeExit( g_pFullFileSystem->Close( hFileHandle ); );
	m_nAddonSize = g_pFullFileSystem->Size( hFileHandle );
	if ( m_nAddonSize == 0 )
	{
		Warning( "Not loading addon '%s' - file is empty?\n", m_strFileName.c_str() );
		return false;
	}

	// RaphaelIT7:
	// Where did GMod pull this value from?
	// I'd guess its the GMA header...
	if ( m_nAddonSize <= 0x24 )
	{
		Warning( "Not loading addon '%s' - file is too small?\n", m_strFileName.c_str() );
		return false;
	}

	static Bootil::AutoBuffer buffer;
	// RaphaelIT7:
	// Kinda feels like a waste of memory usage.
	// Though also then- this is only the minimum size.
	// I wonder, if GMod has to read a 2GB addon, will it resize down?
	buffer.EnsureCapacity( 0xA00000u );
	buffer.SetWritten( ( buffer.GetSize() >= m_nAddonSize ) ? m_nAddonSize : buffer.GetSize() );
	buffer.SetPos( 0 );

	g_pFullFileSystem->Read( buffer.GetBase(), buffer.GetWritten(), hFileHandle );

	Addon::Header header = buffer.ReadType<Addon::Header>();
	if ( header.Ident[0] != 'G' || header.Ident[1] != 'M' || header.Ident[2] != 'A' || header.Ident[3] != 'D' )
	{
		Warning( "Not loading addon '%s' - addon header invalid\n", m_strFileName.c_str() );
		return false;
	}

	m_nGMAVersion = header.Version;
	return ReadAddonFile( hFileHandle, buffer );
}

// RaphaelIT7:
// Small note- GMod calls Close for the FileHandle in here but in our implementation we let
// RunCodeAtScopeExit from the OpenFile take care of it
bool Addon::Reader::ReadAddonFile( FileHandle_t hFileHandle, Bootil::Buffer &buffer )
{
	if ( buffer.GetWritten() <= buffer.GetPos() + 16 )
		return false;

	m_nSteamID = buffer.ReadType<uint64_t>();
	m_nTimestamp = buffer.ReadType<uint64_t>();

	// RaphaelIT7:
	// Ancient! Version 1 addons did not have required content
	if ( m_nGMAVersion > 1 )
	{
		if ( !ReadRequiredContent( buffer ) )
		{
			Warning( "Not loading addon '%s' - couldn't read required addons!\n", m_strFileName.c_str() );
			return false;
		}
	}

	m_strAddonTitle = buffer.ReadString();
	m_strAddonDesc = buffer.ReadString();
	m_strAddonAuthor = buffer.ReadString();
	m_nAddonVersion = buffer.ReadType<int32_t>();

	while ( ReadFileEntry( buffer ) ) {}

	if ( m_bBadAddon )
		return false;

	m_nContentsOffset = buffer.GetPos();
	return BuildOffsets();
}

// RaphaelIT7:
// Unused in GMod
// After Version 1 you are able to specify a list of files required to exist for an addon to work
// Yet GMad does not use it. And GMod does not even keep track.
// NOTE: GMod simply calls ReadString() until it's empty and doesn't even store it!
bool Addon::Reader::ReadRequiredContent( Bootil::Buffer &buffer )
{
	while ( true )
	{
		if ( buffer.GetPos() >= buffer.GetWritten() )
			return false;

		if ( buffer.ReadString().empty() )
			break;
	}

#if 0
	while ( true )
	{
		if ( buffer.GetPos() >= buffer.GetWritten() )
			return false;

		std::string strRequiredFile = buffer.ReadString();
		if ( strRequiredFile.empty() )
			break;

		m_RequiredContent.push_back( strRequiredFile );
	}
#endif

	return true;
}

bool Addon::Reader::ReadFileEntry( Bootil::Buffer &buffer )
{
	if ( buffer.GetWritten() <= buffer.GetPos() + sizeof( uint32_t ) )
	{
		Warning( "Not loading addon '%s', failed to parse addon file list!\n", m_strFileName.c_str() );
		m_bBadAddon = true;
		return false;
	}

	uint32_t iFileNumber = buffer.ReadType<uint32_t>();
	if ( iFileNumber == 0 )
		return false;

	if ( buffer.GetWritten() <= buffer.GetPos() + 1 )
	{
		Warning( "Not loading addon '%s', failed to parse addon file list!\n", m_strFileName.c_str() );
		m_bBadAddon = true;
		return false;
	}

	std::string strFileName = buffer.ReadString();
	if ( buffer.GetWritten() <= buffer.GetPos() + sizeof( uint64_t ) + sizeof( uint32_t ) )
	{
		Warning( "Not loading addon '%s', failed to parse addon file list!\n", m_strFileName.c_str() );
		m_bBadAddon = true;
		return false;
	}

	uint64_t iSize = buffer.ReadType<uint64_t>();
	uint32_t iCRC  = buffer.ReadType<uint32_t>();

	Bootil::String::Lower( strFileName );
	std::string error = Addon::WhiteList::FilenameErrors( strFileName, iSize );
	if ( !error.empty() )
	{
		Warning( "Not loading addon '%s' - %s\n", m_strFileName.c_str(), error.c_str() );
		m_bBadAddon = true;
		return false;
	}

	Bootil::String::File::FixSlashes( strFileName );

	if ( iSize > 2000000000 )
	{
		Warning("Not loading addon '%s' - %s is too large! (%s)\n", m_strFileName.c_str(), m_strFileName.c_str(), Bootil::String::Format::Memory( iSize ).c_str() );
		m_bBadAddon = true;
		return false;
	}

	FileEntry fileEntry;
	fileEntry.strName = std::move( strFileName );
	fileEntry.iSize = iSize;
	fileEntry.iCRC = iCRC;
	fileEntry.iFileNumber = iFileNumber;
	fileEntry.iOffset = 0;

	m_Files.push_back( std::move( fileEntry ) );
	return true;
}

void Addon::Reader::ExtractFiles()
{
	for ( auto &fileEntry : m_Files )
	{
		std::string strOutputPath = "cache/workshop/" + fileEntry.strName;
		if ( Bootil::String::Test::StartsWith( fileEntry.strName, "gamemodes/" ) )
		{
			size_t firstSlash = fileEntry.strName.find( '/', 10 );
			if ( firstSlash != std::string::npos && fileEntry.strName.find( "/content/", firstSlash ) == firstSlash )
			{
				// Strip "content/"
				strOutputPath = "cache/workshop/" + fileEntry.strName.substr( firstSlash + 9 );
			}
		}

		if ( !ShouldBeExtracted( fileEntry.strName ) )
			continue;

		bool bExistsInMod = g_pFullFileSystem->FileExists( strOutputPath.c_str(), "MOD" );
		if ( bExistsInMod == g_pFullFileSystem->FileExists( fileEntry.strName.c_str(), "workshop" ) )
			continue;

		CUtlBuffer contentBufffer;
		if ( !g_pFullFileSystem->ReadFile( fileEntry.strName.c_str(), "workshop", contentBufffer ) )
		{
			DevWarning( "Couldn't read '%s' - what's up with that?\n", fileEntry.strName.c_str() );
			continue;
		}

		std::string strFolder = strOutputPath;
		Bootil::String::File::StripFilename( strFolder );

		g_pFullFileSystem->CreateDirHierarchy( strFolder.c_str(), "MOD" );

		if ( g_pFullFileSystem->WriteFile( strOutputPath.c_str(), "MOD", contentBufffer ) )
			Msg( "Extracted '%s' from workshop addon\n", fileEntry.strName.c_str() );
		else
			DevWarning( "Couldn't write '%s' - already in use?\n", strOutputPath.c_str() );
	}
}

bool Addon::Reader::BuildOffsets()
{
	uint64_t iContentOffset = m_nContentsOffset;
	for ( Addon::FileEntry &file : m_Files )
	{
		file.iOffset = iContentOffset;
		iContentOffset += file.iSize;

		if ( iContentOffset > m_nAddonSize )
		{
			Warning( "Not loading addon '%s' - Malformed .gma file\n", m_strFileName.c_str() );

			m_bBadAddon = true;
			return false;
		}

		if ( iContentOffset > 0xFFFFFFFFULL )
		{
			Warning( "Not loading addon '%s' - file size too large (above 4GB)\n", m_strFileName.c_str() );
			m_bBadAddon = true;
			return false;
		}
	}

	return true;
}

FileHandle_t Addon::Reader::GetPackFile()
{
	return g_pFullFileSystem->Open( m_strFileName.c_str(), "rb", "MOD" );
}

uint32_t Addon::Reader::GetNumFiles()
{
	return m_Files.size();
}

Addon::FileEntry &Addon::Reader::GetFile( uint32_t nFileID )
{
	auto it = m_Files.begin();
	std::advance( it, nFileID );
	return *it;
}