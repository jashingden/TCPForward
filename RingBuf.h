
#pragma once

#include <cstddef>

class CBSRingBuffer
{
public:
    CBSRingBuffer();
    virtual ~CBSRingBuffer();

public:
    // Create with nByteSize
    bool Create(int nByteSize, bool bOverlap = true);

    // Destroy
    void Destroy();

    // Read
    std::size_t Read(char* pDest, std::size_t nBytes, bool bPurge = true);
    void Purge(std::size_t nByte);

    // Write: can overrun
    std::size_t Write(const char* pSrc, std::size_t nBytes, bool& isOverrun);

    // Queued size (that has not been read)
    std::size_t GetQueuedByteCount();

    // Free size (that can be writen before overrun occurs)
    std::size_t GetFreeByteCount();

    // Readjust buffer size
    //
    bool AdjustBufferSize(std::size_t nNewSize);

    // Get buffer allocation size
    //
    std::size_t GetBufferSize();

protected:
    char* m_pBuffer;        // head of buffer
    std::size_t  m_nSize;          // buffer size
    //const char* m_pReadPtr;       // read pointer
    //const char* m_pWritePtr;      // write pointer
    std::size_t  m_nReadPtr;
    std::size_t  m_nWritePtr;
    bool  m_bOverlap;       // can overlap or not
};
