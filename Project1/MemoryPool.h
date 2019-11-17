#pragma once

class MemoryPool
{

public:

	MemoryPool();
	~MemoryPool();

	bool Initialize(unsigned int ChunkByteSize, unsigned int NumOfChunks);
	void Destroy();

	void* Alloc();
	void FreeMem(void* Mem);

	inline unsigned int GetChunkSize() { return m_ChunkSize; }

	inline void SetAllowResize(bool bAllow) { m_bAllowResize = bAllow; }

private:

	unsigned char** m_RawMemoryArray;
	unsigned char* m_Front;

	unsigned int m_ChunkSize;
	unsigned int m_NumOfChunks;

	unsigned int m_MemSize;

	bool m_bAllowResize;

	void Reset();

	bool GrowMemoryArray();
	unsigned char* AllocateNewMemBlock();

	unsigned char* GetNext(unsigned char*& Block);
	void SetNext(unsigned char* BlockToChange, unsigned char* NewNext);

	MemoryPool(const MemoryPool& MemPool) {};
};