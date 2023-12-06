#include "TimeProfiler.h"
std::vector<TimeData> TimeProfiler::TimeProfilerData = std::vector<TimeData>();
mutex TimeLock;

bool TimeProfiler::bEntryExists(string FuncName)
{

	if (TimeProfilerData.empty()) return false;

	for (int i = 0; i < TimeProfilerData.size(); i++)
	{
		if (TimeProfilerData[i].FunctionName == FuncName) return true;
	}

	return false;
}

void TimeProfiler::AddData(string FuncName, float TimeTaken)
{
	TimeLock.lock();

	if (!bEntryExists(FuncName))
	{
		TimeData temp;
		temp.FunctionName = FuncName;
		temp.AverageTime = TimeTaken;
		temp.TotalTime = TimeTaken;
		temp.TimesCalled = 1;
		TimeProfilerData.push_back(temp);

		TimeLock.unlock();
		return;
	}

	for (int i = 0; i < TimeProfilerData.size(); i++)
	{
		if (TimeProfilerData[i].FunctionName != FuncName) continue;

		TimeData temp = TimeProfilerData[i];
		temp.TimesCalled++;
		temp.TotalTime += TimeTaken;
		temp.AverageTime = temp.TotalTime / temp.TimesCalled;

		TimeProfilerData[i] = temp;
	}

	TimeLock.unlock();
}