#pragma once
#include "ProgramProfiler.h"
#include <cstdlib>
#include <mutex>

mutex GlobalOverrideLock;

void* operator new(size_t size);
void operator delete(void* pMem);