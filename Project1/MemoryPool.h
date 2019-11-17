#pragma once

#ifdef _DEBUG
#include "string"
#endif

/*This class should be used with small frequently
deleted/allocated variables. Big variables should
stick to the new operator.*/
class MemoryPool
{

public:

	//Nothing here
	MemoryPool();
	//Calls Destroy()
	~MemoryPool();

	//Block the copy constructor
	MemoryPool(const MemoryPool& MemPool) = delete;

	/*Prepare the memory of given size and free 
	memory allocated previously.
	If NumOfChunks is too big then there will be
	a lot of unused memory.*/
	bool Initialize(unsigned int ChunkByteSize, unsigned int NumOfChunks);
	//Free allocated memory
	void Destroy();

	/*Get a pointer to prepared memory.
	If the whole allocated memory is used up
	this will automaticly allocate more.*/
	void* Alloc();
	/*Set the given memory as free to use.*/
	void FreeMem(void* Mem);

	inline unsigned int GetChunkSize() { return m_ChunkSize; }

	//Whether this can automaticly allocate more memory.
	inline void SetAllowResize(bool bAllow) { m_bAllowResize = bAllow; }

private:

	//Pointer to the prepared memory.
	unsigned char** m_RawMemoryArray;
	//Front of the prepared memory
	unsigned char* m_Front;

	unsigned int m_ChunkSize;
	unsigned int m_NumOfChunks;

	//Size of the memory taken up by this 
	unsigned int m_MemSize;

	bool m_bAllowResize;

	//Reset variables
	void Reset();

	bool GrowMemoryArray();
	//Allocate new memory block
	unsigned char* AllocateNewMemBlock();

	unsigned char* GetNext(unsigned char*& Block);
	void SetNext(unsigned char* BlockToChange, unsigned char* NewNext);

#ifdef _DEBUG
	std::string m_debugName;
	unsigned long m_allocPeak, m_numAllocs;
#endif
};