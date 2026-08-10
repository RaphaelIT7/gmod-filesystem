#include "AddonFileHandle.h"

Addon::FileHandle::FileHandle( Addon::FileInfo *pInfo )
{
	m_hFileHandle = pInfo->m_hFileHandle;
	m_nOffset = pInfo->m_nOffset;
	m_nSize = pInfo->m_nSize;
	m_nPosition = 0;
}

void Addon::FileHandle::OnPackFileUnmounted(void *pUnknown)
{
}

void Addon::FileHandle::CheckPackFileUnmounted(void *pUnknown)
{
}

int Addon::FileHandle::Read( void *pBuffer, int nSize, int nLength )
{
	int iRemaining = static_cast<int>( m_nSize - m_nPosition );
	if ( nLength < iRemaining )
		iRemaining = nLength;

	if ( nSize < iRemaining )
		iRemaining = nSize;

	if ( nSize < 0 )
	{
		Warning( "Trying to read Workshop file into a negatively sized buffer! This should never happen!\n" );
		return 0;
	}

	if ( iRemaining <= 0 )
		return 0;

	if ( !m_hFileHandle )
		return 0;

	g_pFullFileSystem->Seek( m_hFileHandle, m_nOffset + m_nPosition, FILESYSTEM_SEEK_HEAD );
	g_pFullFileSystem->Read( pBuffer, iRemaining, m_hFileHandle );

	m_nPosition += iRemaining;
	return iRemaining;
}

unsigned int Addon::FileHandle::Seek( int64_t iOffset, int iSeekOrigin )
{
	int64_t nNewPosition;
	switch ( iSeekOrigin )
	{
	case SEEK_SET:
		nNewPosition = iOffset;
		break;

	case SEEK_CUR:
		nNewPosition = m_nPosition + iOffset;
		break;

	case SEEK_END:
		nNewPosition = static_cast<int64_t>( m_nSize ) + iOffset;
		break;

	default:
		nNewPosition = m_nOffset;
		break;
	}

	if ( nNewPosition < 0 )
		nNewPosition = 0;

	if ( nNewPosition > static_cast<int64_t>( m_nSize ) )
		nNewPosition = m_nSize;

	m_nPosition = nNewPosition;
	return (unsigned int)m_nPosition;
}

unsigned int Addon::FileHandle::Tell()
{
	return m_nPosition;
}

unsigned int Addon::FileHandle::Size()
{
	return m_nSize;
}