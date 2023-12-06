#include "QuadTree.h"
#include "TimeProfiler.h"
#include <cmath>

Quadrant::Quadrant(int Depth, Position TopLeft, Position BottomRight, QuadTree* InBase)
{
	TopLeftCorner = TopLeft;
	BottomRightCorner = BottomRight;

	Base = InBase;

	if (Depth > 0)
	{
		int Xmid = (TopLeft.x + BottomRight.x) / 2;
		int Ymid = (TopLeft.y + BottomRight.y) / 2;

		ChildQuadrants.push_back(new Quadrant(Depth - 1, TopLeft, Position(Xmid, Ymid), InBase));
		ChildQuadrants.push_back(new Quadrant(Depth - 1, Position(TopLeft.x, Ymid), Position(Xmid, BottomRight.y), InBase));
		ChildQuadrants.push_back(new Quadrant(Depth - 1, Position(Xmid, TopLeft.y), Position(BottomRight.x, Ymid), InBase));
		ChildQuadrants.push_back(new Quadrant(Depth - 1, Position(Xmid, Ymid), BottomRight, InBase));
	}
	else
		Base->Leaves.push_back(this);
}

bool Quadrant::InQuadrantBounds(Position InPos)
{
	if (InPos.x >= TopLeftCorner.x && InPos.x <= BottomRightCorner.x
		&& InPos.y >= TopLeftCorner.y && InPos.y <= BottomRightCorner.y)
		return true;
	return false;
}

void Quadrant::Insert(Box* InBox)
{
	if (InBox == nullptr)
		return;

	if (VISUALISE_TREE)
	{
		int Xmid = (TopLeftCorner.x + BottomRightCorner.x) / 2;
		int Ymid = (TopLeftCorner.y + BottomRightCorner.y) / 2;

		float Red = ((Xmid - MinX) / (MaxX - MinX));
		float Blue = ((Ymid - MinZ) / (MaxZ - MinZ));
		InBox->Colour = Vec3(Red, 0, Blue);
	}

	Boxes.push_back(InBox);
}

Box* Quadrant::Search(Position InPos)
{
	for (int i = 0; i < Boxes.size(); i++)
	{
		if (Boxes[i] != nullptr)
		{
			if (Boxes[i]->Position.x == InPos.x
				&& Boxes[i]->Position.z == InPos.y)
				return Boxes[i];
		}
	}

	return nullptr;
}

void Quadrant::Update(const float DeltaTime)
{
	int BoxVectorSize = Boxes.size();

	if (BoxVectorSize != 0)
	{
		for (int i = 0; i < BoxVectorSize; i++)
		{
			Box* temp = Boxes[i];

			temp->Update(DeltaTime);

			for (int j = 0; j < BoxVectorSize; j++)
			{
				if (temp == Boxes[j]) continue;
				if (!Boxes[j]) continue;

				if (temp->CheckCollision(*Boxes[j]))
				{
					temp->ResolveCollision(*Boxes[j]);
					break;
				}
			}
		}

		Base->GetRemoveLockMutex().lock();
		CheckBoxesToRemove();
		Base->GetRemoveLockMutex().unlock();
	}
}

void Quadrant::CheckBoxesToRemove()
{
	vector<int> IndexToRemove;
	for (int i = 0; i < Boxes.size(); i++)
	{
		Box* temp = Boxes[i];
		if (!InQuadrantBounds(Position(temp->Position.x, temp->Position.z)))
		{
			if (temp->Position.x < MinX || temp->Position.x > MaxX
				|| temp->Position.z < MinZ || temp->Position.z > MaxZ)
			{
				temp->Velocity.x = -temp->Velocity.x;
				temp->Velocity.z = -temp->Velocity.z;
			}
			else
			{
				Base->BoxesToMove.push_back(temp);
				IndexToRemove.push_back(i);
			}
		}
	}

	for (int i = 0; i < IndexToRemove.size(); i++)
	{
		Boxes.erase(Boxes.begin() + (IndexToRemove[i] - i));
	}
}

//=========================================================

QuadTree::QuadTree(int MaxLeafDepth, bool bIsProgramRunning)
{
	int Xmid = (MinX + MaxX) / 2;
	int Ymid = (MinZ + MaxZ) / 2;

	TopLeftCorner = Position(MinX, MinZ);
	BottomRightCorner = Position(MaxX, MaxZ);

	Boxes.reserve(NUMBER_OF_BOXES);

	if (MaxLeafDepth > 0)
	{
		ChildQuadrants.push_back(new Quadrant(MaxLeafDepth - 1, TopLeftCorner, Position(Xmid, Ymid), this));
		ChildQuadrants.push_back(new Quadrant(MaxLeafDepth - 1, Position(TopLeftCorner.x, Ymid), Position(Xmid, BottomRightCorner.y), this));
		ChildQuadrants.push_back(new Quadrant(MaxLeafDepth - 1, Position(Xmid, TopLeftCorner.y), Position(BottomRightCorner.x, Ymid), this));
		ChildQuadrants.push_back(new Quadrant(MaxLeafDepth - 1, Position(Xmid, Ymid), BottomRightCorner, this));
	}

	bProgramRunning = bIsProgramRunning;
	LeafCopy = Leaves;

	for (int i = 0; i < NUMBER_OF_THREADS; i++)
	{
		Threads.push_back(new thread(&QuadTree::GetJob, this));
	}
}

Box* QuadTree::Search(Position InPos)
{
	if (Leaves.size() != 0)
	{
		for (Quadrant*& Leaf : Leaves)
		{
			return (Leaf->Search(InPos));
		}
	}
}

void QuadTree::MoveBoxes()
{
	for (Box* box : BoxesToMove)
	{
		Insert(box);
	}

	BoxesToMove.clear();
}

void QuadTree::Draw()
{
	for (Box* box : Boxes)
	{
		box->Draw();
	}
}

void QuadTree::Insert(Box* InBox)
{
	if (Leaves.size() != 0)
	{
		for (Quadrant* Leaf : Leaves)
		{
			if (Leaf->InQuadrantBounds(Position(InBox->Position.x, InBox->Position.z)))
				Leaf->Insert(InBox);
		}
	}
}

void QuadTree::AddInitialisedBox(Box* InBox)
{
	Boxes.emplace_back(InBox);
	Insert(InBox);
}

void QuadTree::GetJob()
{
	while (bProgramRunning)
	{
		if (!bBoxesInitialised) continue;

		Quadrant* Job = nullptr;
		UpdateLockMutex.lock();

		//get Leaf to call update on if any left in vector
		if (LeafCopy.size() != 0)
		{
			//get first copy to save time, then remove copy from vector
			Job = LeafCopy[0];
			LeafCopy.erase(LeafCopy.begin());

			UpdatesInProgress.push_back(Job);
		}

		UpdateLockMutex.unlock();

		//Call update function if successfully got a leaf
		if (Job)
		{
			Job->Update(CachedDeltaTime);

			UpdateLockMutex.lock();

			//remove leaf from in progress vector, lock to ensure vector is not resized while removing the leaf
			for (int i = 0; i < UpdatesInProgress.size(); i++)
			{
				if (UpdatesInProgress[i] == Job)
				{
					UpdatesInProgress.erase(UpdatesInProgress.begin() + i);
					break;
				}
			}

			UpdateLockMutex.unlock();
			Job = nullptr;
		}

		//Notify all jobs done
		UpdateLockMutex.lock();
		if (LeafCopy.empty() && UpdatesInProgress.empty())
		{
			UpdatesDone.notify_all();
		}
		UpdateLockMutex.unlock();
	}
}

void QuadTree::Update(const float DeltaTime)
{
#ifndef NDEBUG
	auto start = steady_clock::now();
#endif
	LeafCopy = Leaves;
	CachedDeltaTime = DeltaTime;

	unique_lock<mutex> lock(WaitForUpdates);
	UpdatesDone.wait(lock);

	if (BoxesToMove.size() != 0)
		MoveBoxes();

#ifndef NDEBUG
	duration<float> diff = steady_clock::now() - start;
	TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}