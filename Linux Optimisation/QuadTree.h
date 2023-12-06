#pragma once
#include "Box.h"
#include "Constants.h"
#include <vector>
#include <pthread.h>

struct Position
{
	float x;
	float y;

	Position(float inX, float inY)
	{
		x = inX;
		y = inY;
	}
	Position()
	{
		x = 0; y = 0;
	}

	bool operator==(Position Pos)
	{
		return (x == Pos.x && y == Pos.y);
	}
};

class QuadTree;

class Quadrant
{
public:
	Quadrant(int Depth, Position TopLeft, Position BottomRight, QuadTree* InBase);
	Quadrant()
	{
		TopLeftCorner = Position(0, 0);
		BottomRightCorner = Position(0, 0);
	}

	Position TopLeftCorner;
	Position BottomRightCorner;

	QuadTree* Base = nullptr;
	vector<Quadrant*> ChildQuadrants;

	vector<Box*> Boxes;

	bool InQuadrantBounds(Position InPos);
	virtual void Insert(Box* InBox);
	virtual Box* Search(Position InPos);
	virtual void Update(const float DeltaTime);
	virtual void Draw();

	void CheckBoxesToRemove();
};

class QuadTree : public Quadrant
{
public:
	QuadTree(int MaxLeafDepth);
	void MoveBoxes();
	vector<Box*> BoxesToMove;
	vector<Quadrant*> Leaves;

	void Update(const float DeltaTime) override;
	void Draw() override;
	void Insert(Box* InBox) override;
	void SetBoxesInitialised(bool bInitialised) { bBoxesInitialised = bInitialised;}
	Box* Search(Position InPos) override;

	pthread_mutex_t GetRemoveLockMutex() { return RemoveLockMutex;}

	pthread_t Threads[NUMBER_OF_THREADS];
	vector<Quadrant*> LeafCopy;
	vector<Quadrant*> UpdatesInProgress;
	float CachedDeltaTime;
	bool bBoxesInitialised = false;
	pthread_mutex_t UpdateLockMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t UpdatesDoneMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t WaitForUpdates = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t RemoveLockMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t UpdatesDone = PTHREAD_COND_INITIALIZER;

	static void* GetJobWrapper(void* arg);
	void GetJob();
};