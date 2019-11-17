#include "MemoryPool.h"
#include "cstdlib"

#ifdef _DEBUG
#include "assert.h"
#endif

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
	if (m_RawMemoryArray)//If this pool already allocates memory then destroy it
		Destroy();

	m_ChunkSize = ChunkByteSize;
	m_NumOfChunks = NumOfChunks;

	if (GrowMemoryArray())//Create new memory array
		return true;
	return false;
}

void MemoryPool::Destroy()
{
#ifdef _DEBUG
	std::string str;
	if (m_numAllocs != 0)
		str = "***(" + std::to_string(m_numAllocs) + ") ";
	unsigned long totalNumChunks = m_NumOfChunks * m_MemSize;
	unsigned long wastedMem = (totalNumChunks - m_allocPeak) * m_ChunkSize;
	str += "Destroying memory pool: [" + std::to_string((unsigned long)m_ChunkSize) + "] = " + std::to_string(m_allocPeak) + "/" + std::to_string((unsigned long)totalNumChunks) + " (" + std::to_string(wastedMem) + " bytes wasted)\n";
	printf(str.c_str());
#endif

	for (size_t I = 0; I < m_MemSize; I++)
	{
		free(m_RawMemoryArray[I]);//Free memory taken up by this pool
	}
	free(m_RawMemoryArray);

	Reset();
}

void* MemoryPool::Alloc()
{
	if (!m_Front)//If there is not memory allocated left
	{
		if (!m_bAllowResize)//Check whether we can increase size
			return nullptr;

		if (!GrowMemoryArray())//Check whether we successfully allocate memory
			return nullptr;
	}

#ifdef _DEBUG
	// update allocation reports
	++m_numAllocs;
	if (m_numAllocs > m_allocPeak)
		m_allocPeak = m_numAllocs;
#endif

	unsigned char* FetchedChunk = m_Front;//Returned pointer is current front
	m_Front = GetNext(m_Front);//Current front is not longer free so we go down the linked list

	return (FetchedChunk + CHUNK_HEADER_SIZE);//Return pointer to the actual data space
}

void MemoryPool::FreeMem(void* Mem)
{
	if (Mem)
	{
		unsigned char* Chunk = ((unsigned char*)Mem) - CHUNK_HEADER_SIZE;//Convert pointer so it point to the header

		SetNext(Chunk, m_Front);
		m_Front = Chunk;//Freed memory is now the new front

#ifdef _DEBUG
// update allocation reports
		--m_numAllocs;
		assert(m_numAllocs >= 0);
#endif
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

#ifdef _DEBUG
	m_allocPeak = 0;
	m_numAllocs = 0;
#endif
}

bool MemoryPool::GrowMemoryArray()
{
#ifdef _DEBUG
	std::string str("Growing memory pool: Chunk size: " + std::to_string((unsigned long)m_ChunkSize) + ", Memory size: " + std::to_string((unsigned long)m_MemSize + 1) + "\n");
	printf(str.c_str()); 
#endif

	size_t AllocationSize = sizeof(char*) * (m_MemSize + 1);
	unsigned char** NewMemArray = (unsigned char**)malloc(AllocationSize);

	if (!NewMemArray)//Memory couldn't be allocated
		return false;

	for (size_t I = 0; I < m_MemSize; I++)
		NewMemArray[I] = m_RawMemoryArray[I];//Existing pointers are copied

	NewMemArray[m_MemSize] = AllocateNewMemBlock();//New block of memory is added to the pool

	if (m_Front)
	{
		unsigned char* CurrFront = m_Front;
		unsigned char* Next = GetNext(m_Front);

		while (Next)
		{
			CurrFront = Next;
			Next = GetNext(CurrFront);
		}
		SetNext(CurrFront, NewMemArray[m_MemSize]);//Place the new memory block at the end of the list
	}
	else
	{
		m_Front = NewMemArray[m_MemSize];
	}

	if (m_RawMemoryArray)//Free the memory allocated previously
		free(m_RawMemoryArray);

	m_RawMemoryArray = NewMemArray;
	m_MemSize++;

	return true;
}

unsigned char* MemoryPool::AllocateNewMemBlock()
{
	size_t ChunkSize = CHUNK_HEADER_SIZE + m_ChunkSize;//Actual chunk also has a pointer to another chunk
	size_t BlockSize = ChunkSize * m_NumOfChunks;

	unsigned char* NewBlock = (unsigned char*)malloc(BlockSize);

	if (!NewBlock)//Memory allocation failed
		return nullptr;

	unsigned char* Current = NewBlock;
	unsigned char* End = NewBlock + BlockSize;

	while (Current < End)//Turn allocated memory into a linked list
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