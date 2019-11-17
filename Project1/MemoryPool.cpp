#include "MemoryPool.h"
#include "cstdlib"

#define CHUNK_HEADER_SIZE (sizeof(unsigned char*))

MemoryPool::MemoryPool()
{
	Reset();
}

MemoryPool::~MemoryPool()
{
	Destroy();
}

bool MemoryPool::Initialize(unsigned int ChunkByteSize, unsigned int NumOfChunks)
{
	if (m_RawMemoryArray)
		Destroy();

	m_ChunkSize = ChunkByteSize;
	m_NumOfChunks = NumOfChunks;

	if (GrowMemoryArray())
		return true;
	return false;
}

void MemoryPool::Destroy()
{
	for (size_t I = 0; I < m_MemSize; I++)
	{
		free(m_RawMemoryArray[I]);
	}
	free(m_RawMemoryArray);

	Reset();
}

void* MemoryPool::Alloc()
{
	if (!m_Front)
	{
		if (!m_bAllowResize)
			return nullptr;

		if (!GrowMemoryArray())
			return nullptr;
	}

	unsigned char* FetchedChunk = m_Front;
	m_Front = GetNext(m_Front);

	return (FetchedChunk + CHUNK_HEADER_SIZE);
}

void MemoryPool::FreeMem(void* Mem)
{
	if (Mem)
	{
		unsigned char* Chunk = ((unsigned char*)Mem) - CHUNK_HEADER_SIZE;

		SetNext(Chunk, m_Front);
		m_Front = Chunk;
	}
}

void MemoryPool::Reset()
{
	m_RawMemoryArray = nullptr;
	m_Front = nullptr;
	m_bAllowResize = true;
	m_ChunkSize = 0;
	m_NumOfChunks = 0;
	m_MemSize = 0;
}

bool MemoryPool::GrowMemoryArray()
{
	size_t AllocationSize = sizeof(char*) * (m_MemSize + 1);
	unsigned char** NewMemArray = (unsigned char**)malloc(AllocationSize);

	if (!NewMemArray)
		return false;

	for (size_t I = 0; I < m_MemSize; I++)
		NewMemArray[I] = m_RawMemoryArray[I];

	NewMemArray[m_MemSize] = AllocateNewMemBlock();

	if (m_Front)
	{
		unsigned char* CurrFront = m_Front;
		unsigned char* Next = GetNext(m_Front);

		while (Next)
		{
			CurrFront = Next;
			Next = GetNext(CurrFront);
		}
		SetNext(CurrFront, NewMemArray[m_MemSize]);
	}
	else
	{
		m_Front = NewMemArray[m_MemSize];
	}

	if (m_RawMemoryArray)
		free(m_RawMemoryArray);

	m_RawMemoryArray = NewMemArray;
	m_MemSize++;

	return true;
}

unsigned char* MemoryPool::AllocateNewMemBlock()
{
	size_t ChunkSize = CHUNK_HEADER_SIZE + m_ChunkSize;
	size_t BlockSize = ChunkSize * m_NumOfChunks;

	unsigned char* NewBlock = (unsigned char*)malloc(BlockSize);

	if (!NewBlock)
		return nullptr;

	unsigned char* Current = NewBlock;
	unsigned char* End = NewBlock + BlockSize;

	while (Current < End)
	{
		unsigned char* Next = Current + ChunkSize;

		unsigned char** ChunkHeader = (unsigned char**)Current;
		ChunkHeader[0] = (Next < End) ? Next : nullptr;

		Current += ChunkSize;
	}

	return NewBlock;
}

unsigned char* MemoryPool::GetNext(unsigned char*& Block)
{
	unsigned char** ChunkHeader = (unsigned char**)Block;
	return ChunkHeader[0];
}

void MemoryPool::SetNext(unsigned char* BlockToChange, unsigned char* NewNext)
{
	unsigned char** ChunkHeader = (unsigned char**)BlockToChange;
	ChunkHeader[0] = NewNext;
}