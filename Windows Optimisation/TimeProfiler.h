#pragma once
#ifndef TIMEPROFILER_H
#define TIMEPROFILER_H

#include <string>
#include <mutex>
#include <vector>

using namespace std;

struct TimeData
{
	string FunctionName;
	float TotalTime = 0.0f;
	float AverageTime = 0.0f;
	int TimesCalled = 0;
};

static class TimeProfiler
{
public:
	static vector<TimeData> TimeProfilerData;
	static void AddData(string FuncName, float TimeTaken);
private:
	static bool bEntryExists(string FuncName);
};

#endif