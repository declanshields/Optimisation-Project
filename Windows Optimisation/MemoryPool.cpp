#include "MemoryPool.h"

MemoryPool::MemoryPool(int InBlockSize)
{
	BlockSize = InBlockSize;
	AvailableMemory = new char[(BlockSize + sizeof(Header) + sizeof(Footer)) * MEMORY_POOL_SIZE];
	AvailableBlocks = new char[MEMORY_POOL_SIZE];

	char* Block = &AvailableBlocks[0];

	for (int i = 0; i < MEMORY_POOL_SIZE; ++i)
	{
		AvailableBlocks[i] = *Block;
		Block += (BlockSize + sizeof(Header) + sizeof(Footer));
	}

	Top = MEMORY_POOL_SIZE - 1;
}

char* MemoryPool::Allocate()
{
	PoolLock.lock();

	int TempTop = Top--;

	if (!IsEmpty())
	{
		AvailableBlocks[TempTop] = *InitMemory(&AvailableBlocks[TempTop]);
	}
	else
	{
		char* pMem = (char*)malloc(BlockSize + sizeof(Header) + sizeof(Footer));
		OverflowBlocks.push_back(InitMemory(pMem));
	}

	PoolLock.unlock();

	if (OverflowBlocks.size() != 0)
		return (OverflowBlocks[OverflowBlocks.size()]);
	else
		return &AvailableBlocks[TempTop];
}

char* MemoryPool::InitMemory(char* pMem)
{
	char* tempMem = (char*)pMem;
	Header* pHeader = (Header*)tempMem;
	if (pHeader)
	{
		pHeader->pPrev = nullptr;
		pHeader->pNext = nullptr;
		pHeader->Size = BlockSize;
		if (!TrackerManager::DoesEntryExist("MemoryPoolTracker"))
		{
			pHeader->Tracker = TrackerManager::CreateNewEntry("MemoryPoolTracker");
		}

		pHeader->Tracker = TrackerManager::RetrieveTrackerByName("MemoryPoolTracker");

		if (pHeader->Tracker->StartHeader == nullptr)
		{
			pHeader->Tracker->StartHeader = pHeader;
		}
		else if (pHeader->Tracker->StartHeader->pNext == nullptr)
		{
			pHeader->Tracker->StartHeader->pNext = pHeader;
			pHeader->pPrev = pHeader->Tracker->StartHeader;
			pHeader->Tracker->EndHeader = pHeader;
		}
		else
		{
			pHeader->Tracker->EndHeader->pNext = pHeader;
			pHeader->pPrev = pHeader->Tracker->EndHeader;
			pHeader->Tracker->EndHeader = pHeader;
		}

		void* pFooterAddr = tempMem + sizeof(Header) + BlockSize;
		Footer* pFooter = (Footer*)pFooterAddr;

		pHeader->CheckValue = HEADERCHECKVALUE;
		pFooter->CheckValue = FOOTERCHECKVALUE;

		pHeader->Tracker->AddBytesAllocated(BlockSize + sizeof(Header) + sizeof(Footer));
	}

	return tempMem;
}

void MemoryPool::Free(void* pMem)
{
	if (OverflowBlocks.size() != 0)
	{
		for (int i = 0; i < OverflowBlocks.size(); i++)
		{
			if (OverflowBlocks[i] != pMem) continue;
			else
			{
				OverflowBlocks.erase(OverflowBlocks.begin() + i);
				return;
			}
		}
	}
	
	AvailableBlocks[++Top] = *(char*)pMem;
}