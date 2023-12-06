#pragma once
#include "Box.h"
#include "Constants.h"
#include <vector>

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

	void CheckBoxesToRemove();

private:
	void UpdateFromThread(const float DeltaTime);
};

class QuadTree : public Quadrant
{
public:
	QuadTree(int MaxLeafDepth, bool bIsProgramRunning);

	vector<Box*> BoxesToMove;
	vector<Quadrant*> Leaves;

	void Update(const float DeltaTime) override;
	void Draw();
	void Insert(Box* InBox)            override;
	Box* Search(Position InPos)        override;
	void MoveBoxes();

	void SetProgramRunning(bool bIsProgramRunning) { bProgramRunning = bIsProgramRunning; }

	mutex& GetRemoveLockMutex() { return RemoveLock; }

	void AddInitialisedBox(Box* InBox);

	void SetBoxesInitialised(bool bInitialised) { bBoxesInitialised = bInitialised; }

private:
	//Vector to store leaves and a copy of the leaves
	//The vector of copies is used for the threads, so that it can be removed to track how many leaves are left,
	//without deleting the leaves themselves
	vector<Quadrant*> LeafCopy;
	vector<thread*> Threads;
	vector<Quadrant*> UpdatesInProgress;

	bool bProgramRunning = false;
	float CachedDeltaTime;

	mutex UpdateLockMutex;
	mutex UpdatesDoneMutex;
	mutex WaitForUpdates;
	mutex RemoveLock;
	condition_variable UpdatesDone;

	bool bBoxesInitialised = false;

	void GetJob();
};