#include "GlobalOperatorOverride.h"
#include <stdexcept>
#include <iostream>

void* operator new(size_t size)
{
	GlobalOverrideLock.lock();

	size_t nRequestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(nRequestedBytes);
	Header* pHeader = (Header*)pMem;

	if (pHeader)
	{
		pHeader->pPrev = nullptr;
		pHeader->pNext = nullptr;
		pHeader->Size = size;
		if (!TrackerManager::DoesEntryExist("VariableTracker"))
		{
			pHeader->Tracker = TrackerManager::CreateNewEntry("VariableTracker");
		}

		pHeader->Tracker = TrackerManager::RetrieveTrackerByName("VariableTracker");

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
		
		void* pFooterAddr = pMem + sizeof(Header) + size;
		Footer* pFooter = (Footer*)pFooterAddr;

		pHeader->CheckValue = HEADERCHECKVALUE;
		pFooter->CheckValue = FOOTERCHECKVALUE;

		pHeader->Tracker->AddBytesAllocated(nRequestedBytes);
	}

	void* pStartMemBlock = pMem + sizeof(Header);
	GlobalOverrideLock.unlock();

	return pStartMemBlock;
}

void operator delete (void* pMem)
{
	GlobalOverrideLock.lock();

	if (!pMem)
	{
		GlobalOverrideLock.unlock();
		return;
	}

	Header* pHeader = (Header*)((char*)pMem - sizeof(Header));
	Footer* pFooter = (Footer*)((char*)pMem + pHeader->Size);

	if (pHeader != nullptr)
	{
		pHeader->Tracker->RemoveBytesAllocated(sizeof(Header) + pHeader->Size + sizeof(Footer));
		if (pHeader == pHeader->Tracker->StartHeader)
		{
			if (pHeader->pNext)
			{
				pHeader->Tracker->StartHeader = pHeader->pNext;
				pHeader->pNext->pPrev = nullptr;
			}
		}
		if (pHeader == pHeader->Tracker->EndHeader)
		{
			if (pHeader->pPrev)
			{
				pHeader->Tracker->EndHeader = pHeader->pPrev;
				pHeader->pPrev->pNext = nullptr;
			}
		}

		if (pHeader->pNext && pHeader->pPrev)
		{
			pHeader->pNext->pPrev = pHeader->pPrev;
			pHeader->pPrev->pNext = pHeader->pNext;
		}

		if (!pHeader->isValid())
		{
			std::cout << "Header check value at memory address: " << pHeader << " is incorrect\n";
		}
	}

	if (!pFooter->isValid())
	{
		if (pFooter->CheckValue != FOOTERCHECKVALUE)
		{
			std::cout << "Footer check value at memory address: " << pFooter << " is incorrect\n";
		}
	}

	free(pHeader);

	GlobalOverrideLock.unlock();
}