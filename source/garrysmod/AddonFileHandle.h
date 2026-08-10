#pragma once

#include "filesystem.h"
#include <string>

namespace Addon
{

struct FileInfo
{
	std::string m_strFileName;
	std::string m_strFullFileName;
	int64_t m_nSize;
	int64_t m_nOffset;
	FileHandle_t m_hFileHandle;
	uint64_t m_nWsid;
};

class FileHandle
{
public:
	FileHandle( Addon::FileInfo* pInfo );

	// Called from Addon::FileSystem::Clear & Addon::FileSystem::UnmountPackFile
	void OnPackFileUnmounted( void *pUnknown );

	// Unused? Perhaps used on the client...
	void CheckPackFileUnmounted( void *pUnknown );

	// Called from the CFileHandle
	int Read( void *pBuffer, int nDestSize, int nLength );
	unsigned int Seek( int64 nOffset, int nWhence );
	unsigned int Tell();
	unsigned int Size();

	// Just... deletes itself? Also seems unused...
	void Release();

private:
	FileHandle_t m_hFileHandle;
	uint64_t m_nSize;
	uint64_t m_nOffset;
	int64_t m_nPosition;
};

}