#pragma once

#include "tier0/platform.h"

namespace Addon
{

struct FileInfo
{
	const char* m_pszFileName;
};

class FileHandle
{
public:
	FileHandle(Addon::FileInfo* pInfo);

	// Called from Addon::FileSystem::Clear & Addon::FileSystem::UnmountPackFile
	void OnPackFileUnmounted(void* pUnknown);

	// Unused? Perhaps used on the client...
	void CheckPackFileUnmounted(void* pUnknown);

	// Called from the CFileHandle
	int Read( void* pBuffer, int nDestSize, int nLength );
	unsigned int Seek( int64 nOffset, int nWhence );
	unsigned int Tell();
	unsigned int Size();

	// Just... deletes itself? Also seems unused...
	void Release();
};

}