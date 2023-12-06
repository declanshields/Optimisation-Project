#pragma once

#include "Constants.h"
#include "ProgramProfiler.h"
#include <mutex>

class MemoryPool
{
public:
	MemoryPool(int InBlockSize);

	char* Allocate();
	void Free(void* pMem);
	char* InitMemory(char* pMem);
	bool IsEmpty() const { return Top < 0; }
	bool IsFull() const { return Top == (MEMORY_POOL_SIZE - 1); }

	char* AvailableMemory;
	char* AvailableBlocks;

	vector<char*> OverflowBlocks;
	int Top;

private:
	mutex PoolLock;
	int BlockSize;
};

