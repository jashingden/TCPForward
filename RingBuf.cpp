//
// RingBuf.cpp
//
#include "RingBuf.h"
#include <string.h>
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//

CBSRingBuffer::CBSRingBuffer()
{
    m_pBuffer = NULL;//m_pReadPtr = m_pWritePtr = NULL;
    m_nReadPtr = 0;
    m_nWritePtr = 0;
    m_nSize = 0;
}

CBSRingBuffer::~CBSRingBuffer()
{
    Destroy();
}

bool CBSRingBuffer::Create(int nByteSize, bool bOverlap)
{
	if (m_pBuffer != NULL) {
		m_nReadPtr = 0;
		m_nWritePtr = 0;
		return true;
	}

    try
    {
        m_pBuffer = new char[nByteSize];
    }
    catch (...)
    {
        return false;
    }

    m_nSize = nByteSize;
    memset(m_pBuffer, 0, nByteSize);
    //m_pReadPtr = m_pBuffer;
    //m_pWritePtr = m_pBuffer;
    m_nReadPtr = 0;
    m_nWritePtr = 0;
    m_bOverlap = bOverlap;
    return true;
}

void CBSRingBuffer::Destroy()
{
	if (m_pBuffer)
		delete [] m_pBuffer;
    m_pBuffer = NULL;
    m_nSize = 0;
    //m_pReadPtr = m_pWritePtr = NULL;
    m_nReadPtr = 0;
    m_nWritePtr = 0;
}

size_t CBSRingBuffer::Read(char* pDest, size_t nBytes, bool bPurge)
{
    if (m_pBuffer == NULL)
        return 0;

    size_t nBytesInBuffer = GetQueuedByteCount();
    size_t nLength = min(nBytes, nBytesInBuffer);
    if (nLength <= 0)
        return 0;

    // first chunk: before wrap around
    //size_t nRight = m_pBuffer + m_nSize - m_pReadPtr;
    size_t nRight = m_nSize - m_nReadPtr;
    assert(0 <= nRight && nRight <= m_nSize);
    size_t nFirstChunk = min(nLength, nRight);
    if (nFirstChunk > 0)
    {
        //memcpy(pDest, m_pReadPtr, nFirstChunk);
        memcpy(pDest, m_pBuffer + m_nReadPtr, nFirstChunk);
        if (bPurge)
        {
            //m_pReadPtr += nFirstChunk;
            m_nReadPtr += nFirstChunk;
            if (m_nReadPtr >= m_nSize)
            {
                m_nReadPtr = 0;
            }
        }
    }

    // second chunk
    size_t nLeft = nLength - nFirstChunk;
    if (nLeft > 0)
    {
        memcpy(pDest + nFirstChunk, m_pBuffer, nLeft);
        if (bPurge)
        {
            //m_pReadPtr = m_pBuffer + nLeft;
            m_nReadPtr = nLeft;
        }
    }

    //assert(m_pReadPtr >= m_pBuffer);
    //assert(m_pReadPtr <= (m_pBuffer + m_nSize));
    assert(m_nReadPtr >= 0);
    assert(m_nReadPtr < m_nSize);

    return nLength;
}

void CBSRingBuffer::Purge(size_t nBytes)
{
    if (m_pBuffer == NULL)
        return;

    size_t nBytesInBuffer = GetQueuedByteCount();
    size_t nLength = min(nBytes, nBytesInBuffer);

    if (nLength <= 0)
    {
        //m_pReadPtr = m_pWritePtr;  //Clear all data
        m_nReadPtr = 0;
        m_nWritePtr = 0;
    }
    else
    {
        // first chunk: before wrap around
        //size_t nRight = m_pBuffer + m_nSize - m_pReadPtr;
        size_t nRight = m_nSize - m_nReadPtr;
        size_t nFirstChunk = min(nLength, nRight);
        m_nReadPtr += nFirstChunk;

        // second chunk
        int nLeft = nLength - nFirstChunk;
        if (nLeft > 0)
            m_nReadPtr = nLeft;

        if (m_nReadPtr >= m_nSize)
        {
            m_nReadPtr = 0;
        }
    }

    assert(m_nReadPtr >= 0);
    assert(m_nReadPtr < m_nSize);
}

size_t CBSRingBuffer::Write(const char* pSrc, size_t nBytes, bool& isOverrun)
{
	isOverrun = false;
    if (m_pBuffer == NULL)
        return 0;

	size_t nMax = GetFreeByteCount();
	if (m_bOverlap)
	{
		if (nBytes > nMax)
			isOverrun = true;
		nMax = nBytes;
	}
    nMax = min(nMax, m_nSize);

    // first chunk: before wrap around
    size_t nRight = m_nSize - m_nWritePtr;
    assert(0 <= nRight && nRight <= m_nSize);
    size_t nFirstChunk = min(min(nMax, nRight), nBytes);
    if (nFirstChunk > 0)
    {
        memcpy(m_pBuffer + m_nWritePtr, pSrc, nFirstChunk);
        m_nWritePtr += nFirstChunk;
        if (m_nWritePtr >= m_nSize)
        {
            m_nWritePtr = 0;
        }
    }

    // second chunk
    int nLeft = min(nBytes, nMax) - nFirstChunk;
    if (nLeft <= 0)
    {
        return nFirstChunk;
    }

    memcpy(m_pBuffer, pSrc + nFirstChunk, nLeft);
    m_nWritePtr = nLeft;

    assert(m_nWritePtr >= 0);
    assert(m_nWritePtr < m_nSize);


    return nFirstChunk + nLeft;
}

size_t CBSRingBuffer::GetQueuedByteCount()
{
    if (m_pBuffer == NULL)
        return 0;

    size_t nLength;

    if (m_nWritePtr >= m_nReadPtr)
    {
        nLength = m_nWritePtr - m_nReadPtr;
    }
    else
    {
        nLength = m_nWritePtr + m_nSize - m_nReadPtr;
    }
    assert(nLength <= m_nSize);
    return nLength;
}

size_t CBSRingBuffer::GetFreeByteCount()
{
    if (m_pBuffer == NULL)
        return 0;

    size_t nLength;

    if (m_nReadPtr > m_nWritePtr)
    {
        nLength = m_nReadPtr - m_nWritePtr - 1;
    }
    else
    {
        nLength = m_nReadPtr + m_nSize - m_nWritePtr - 1;
    }
    assert(nLength < m_nSize);
    return nLength;
}

bool CBSRingBuffer::AdjustBufferSize(size_t nNewSize)
{
    if (nNewSize == GetBufferSize())
        return true;

    char* pNewBuffer;
    try
    {
        pNewBuffer = new char [nNewSize];
    }
    catch (...)
    {
        return false;
    }

    // Populate new buffer with existing contents
    //
    size_t nBytesInBuffer = GetQueuedByteCount();
    size_t nCopyLength = min(nNewSize, nBytesInBuffer);
    if (nCopyLength > 0)
    {
        // first chunk: before wrap around
        //
        size_t nRight = m_nSize - m_nReadPtr;
        assert(0 <= nRight && nRight <= m_nSize);
        size_t nFirstChunk = min(nCopyLength, nRight);
        if (nFirstChunk > 0)
            memcpy(pNewBuffer, m_pBuffer + m_nReadPtr, nFirstChunk);

        // second chunk
        size_t nLeft = nCopyLength - nFirstChunk;
        if (nLeft > 0)
            memcpy(pNewBuffer + nFirstChunk, m_pBuffer, nLeft);
    }
    m_nReadPtr = 0;
    m_nWritePtr = nCopyLength;
    if (m_nWritePtr >= nNewSize)
        m_nWritePtr = 0;

	if (m_pBuffer)
		delete []m_pBuffer;
    m_pBuffer = pNewBuffer;
	m_nSize = nNewSize;

    return true;
}

size_t CBSRingBuffer::GetBufferSize()
{
    return m_nSize;
}

