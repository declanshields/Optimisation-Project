#include "QuadTree.h"
#include "TimeProfiler.h"
#include <cmath>
#include <assert.h>

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
	auto start = steady_clock::now();

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

	duration<float> diff = steady_clock::now() - start;
	TimeProfiler::AddData(__FUNCTION__, diff.count());
}

Box* Quadrant::Search(Position InPos)
{
	auto start = steady_clock::now();

	for (int i = 0; i < Boxes.size(); i++)
	{
		if (Boxes[i] != nullptr)
		{
			if (Boxes[i]->Position.x == InPos.x
				&& Boxes[i]->Position.z == InPos.y)
				return Boxes[i];
		}
	}

	duration<float> diff = steady_clock::now() - start;
	TimeProfiler::AddData(__FUNCTION__, diff.count());

	return nullptr;
}

void Quadrant::Update(const float DeltaTime)
{
	int BoxVectorSize = Boxes.size();
	vector<Box*> temp;
	temp.reserve(BoxVectorSize);
	temp = Boxes;

	if (BoxVectorSize != 0)
	{
		for (int i = 0; i < BoxVectorSize; i++)
		{
			Box* TempBox = temp[i];

			TempBox->Update(DeltaTime);

			for (int j = 0; j < BoxVectorSize; j++)
			{
				if (TempBox == temp[j]) continue;
				if (!temp[j]) continue;

				if (TempBox->CheckCollision(*temp[j]))
				{
					TempBox->ResolveCollision(*temp[j]);
					break;
				}
			}
		}
	}

	pthread_mutex_t tempLock = Base->GetRemoveLockMutex();
	pthread_mutex_lock(&tempLock);
	CheckBoxesToRemove();
	pthread_mutex_unlock(&tempLock);
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
			else{
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

void Quadrant::Draw()
{
	for (Box* box : Boxes)
	{
		box->Draw();
	}
}

//=========================================================

QuadTree::QuadTree(int MaxLeafDepth)
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

	LeafCopy = Leaves;

	bool bSuccess = true;
	bSuccess &= (0 == pthread_mutex_init(&UpdateLockMutex, NULL));
	bSuccess &= (0 == pthread_mutex_init(&UpdatesDoneMutex, NULL));
	bSuccess &= (0 == pthread_mutex_init(&WaitForUpdates, NULL));
	bSuccess &= (0 == pthread_mutex_init(&RemoveLockMutex, NULL));
	assert(bSuccess == true);

	for (int i = 0; i < NUMBER_OF_THREADS; i++)
	{
		pthread_create(&Threads[i], NULL, &QuadTree::GetJobWrapper, this);
	}
}

Box* QuadTree::Search(Position InPos)
{
	if (Leaves.size() != 0)
	{
		for (Quadrant* Leaf : Leaves)
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
	if (Leaves.size() != 0)
	{
		for (Quadrant* Leaf : Leaves)
		{
			Leaf->Draw();
		}
	}
	else
	{
		for (Box* box : Boxes)
		{
			box->Draw();
		}
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
	else
	{
		Boxes.push_back(InBox);
	}
}

void* QuadTree::GetJobWrapper(void* arg)
{
	QuadTree* Tree = reinterpret_cast<QuadTree*>(arg);
	Tree->GetJob();
	return NULL;
}

void QuadTree::GetJob()
{
	while(true)
	{
		if (!bBoxesInitialised) continue;

		Quadrant* Job = nullptr;
		pthread_mutex_lock(&UpdateLockMutex);

		if (LeafCopy.size() != 0)
		{
			Job = LeafCopy[0];
			LeafCopy.erase(LeafCopy.begin());

			UpdatesInProgress.push_back(Job);
		}

		pthread_mutex_unlock(&UpdateLockMutex);

		if (Job)
		{
			Job->Update(CachedDeltaTime);

			pthread_mutex_lock(&UpdateLockMutex);
			for(int i = 0; i < UpdatesInProgress.size(); i++)
			{
				if (UpdatesInProgress[i] == Job)
				{
					UpdatesInProgress.erase(UpdatesInProgress.begin() + i);
					break;
				}
			}

			pthread_mutex_unlock(&UpdateLockMutex);
			Job = nullptr;
		}

		pthread_mutex_lock(&UpdateLockMutex);
		if (LeafCopy.empty() && UpdatesInProgress.empty())
		{
			pthread_cond_signal(&UpdatesDone);
		}
		pthread_mutex_unlock(&UpdateLockMutex);
	}
}

void QuadTree::Update(const float DeltaTime)
{
	auto start = steady_clock::now();

	LeafCopy = Leaves;
	CachedDeltaTime = DeltaTime;

	pthread_cond_wait(&UpdatesDone, &WaitForUpdates);

	if (BoxesToMove.size() != 0)
		MoveBoxes();

	duration<float> diff = steady_clock::now() - start;
	TimeProfiler::AddData(__FUNCTION__, diff.count());
}