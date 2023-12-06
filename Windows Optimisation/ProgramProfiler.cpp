#include "ProgramProfiler.h"
#include <iostream>
#include <cstdlib>

int TrackerManager::AvailableTrackerIndex = 0;
NamedTracker TrackerManager::Trackers[NUMOFTRACKERS];

void* BaseTracker::operator new(size_t size)
{
	char* pMem = (char*)malloc(sizeof(BaseTracker));

	return pMem;
}

bool TrackerManager::DoesEntryExist(const char* NewTrackerName)
{
	bool bTrackerExists = false;

	for (int i = 0; i < NUMOFTRACKERS; i++)
	{
		if (NewTrackerName == Trackers[i].TrackerName)
		{
			bTrackerExists = true;
			break;
		}
	}

	return bTrackerExists;
}

BaseTracker* TrackerManager::RetrieveTrackerByName(const char* TrackerName)
{
	for (int i = 0; i < NUMOFTRACKERS; i++)
	{
		if (Trackers[i].TrackerName == TrackerName)
			return Trackers[i].Tracker;
	}
}

BaseTracker* TrackerManager::CreateNewEntry(const char* NewTrackerName)
{
	BaseTracker* Tracker = (BaseTracker*)((char*)malloc(sizeof(BaseTracker)));
	Tracker->AllocatedBytes = 0;
	Tracker->AllocatedBytes += sizeof(BaseTracker);
	Tracker->StartHeader = nullptr;
	Tracker->EndHeader = nullptr;

	NamedTracker tempTracker;
	tempTracker.TrackerName = NewTrackerName;
	tempTracker.Tracker = Tracker;
	Trackers[AvailableTrackerIndex] = tempTracker;
	AvailableTrackerIndex++;

	return Tracker;
}

void TrackerManager::WalkTheHeap()
{
	for (int i = 0; i < NUMOFTRACKERS; i++)
	{
		if (Trackers[i].Tracker)
		{
			cout << "Tracker Name: " << Trackers[i].TrackerName << "\n";
			Trackers[i].Tracker->WalkTheHeap();
		}
	}
}

void BaseTracker::WalkTheHeap()
{
	Header* CurrentHeader = StartHeader;
	while (CurrentHeader != nullptr)
	{
		cout << "Header Address: " << CurrentHeader << "\n";
		cout << "  -Previous Header Address: " << CurrentHeader->pPrev << "\n";
		cout << "  -Next Header Address: " << CurrentHeader->pNext << "\n";
		cout << "  -Memory Size: " << CurrentHeader->Size << "\n";
		if (!CurrentHeader->isValid())
		{
			cout << "Header check value at memory address: " << CurrentHeader << " is incorrect\n";
			break;
		}
		CurrentHeader = CurrentHeader->pNext;
	}

	cout << "Total Allocated Memory: " << AllocatedBytes << "\n";
}