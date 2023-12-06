#pragma once

#ifndef PROGRAMPROFILER_H
#define PROGRAMPROFILER_H

#define HEADERCHECKVALUE 0xDEADC0DE
#define FOOTERCHECKVALUE 0xFEEDC0DE
#define NUMOFTRACKERS 5

#include <vector>
#include <string>

using namespace std;

struct Header;

class BaseTracker
{
public:
	void* operator new(size_t size);

	int AllocatedBytes = 0;
	Header* StartHeader = nullptr;
	Header* EndHeader = nullptr;

	inline void AddBytesAllocated(int NumberOfBytes) { AllocatedBytes += NumberOfBytes; }
	inline void RemoveBytesAllocated(int NumberOfBytes) { AllocatedBytes -= NumberOfBytes; }
	void WalkTheHeap();
};

struct NamedTracker
{
	const char* TrackerName;
	BaseTracker* Tracker;
};

struct Header
{
	int Size;
	int CheckValue;
	BaseTracker* Tracker;
	Header* pNext;
	Header* pPrev;

	bool isValid()
	{
		bool bValid;
		(CheckValue == HEADERCHECKVALUE) ? bValid = true : bValid = false;
		return bValid;
	}
};

struct Footer
{
	int Reserved;
	int CheckValue;

	bool isValid()
	{
		bool bValid;
		(CheckValue == FOOTERCHECKVALUE) ? bValid = true : bValid = false;
		return bValid;
	}
};

static class TrackerManager
{
public:
	static int AvailableTrackerIndex;
	static NamedTracker Trackers[NUMOFTRACKERS];
	static bool DoesEntryExist(const char* NewTrackerName);
	static BaseTracker* CreateNewEntry(const char* NewTrackerName);
	static BaseTracker* RetrieveTrackerByName(const char* TrackerName);

	static void WalkTheHeap();
};

#endif