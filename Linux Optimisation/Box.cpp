#include "Box.h"

#include "ProgramProfiler.h"
#include "TimeProfiler.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include "Constants.h"

void Vec3::Normalise()
{
	float Length = std::sqrt(x * x + y * y + z * z);
	if (Length != 0)
	{
		x /= Length;
		y /= Length;
		z /= Length;
	}
}

mutex BoxCreateLock;

void* Box::operator new(size_t size)
{
	BoxCreateLock.lock();

	size_t nRequestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(nRequestedBytes);
	void* pStartMemBlock;

#ifndef NDEBUG
	Header* pHeader = (Header*)pMem;

	if (pHeader)
	{
		pHeader->pPrev = nullptr;
		pHeader->pNext = nullptr;
		pHeader->Size = size;

		if (!TrackerManager::DoesEntryExist("BoxTracker"))
		{
			pHeader->Tracker = TrackerManager::CreateNewEntry("BoxTracker");
		}

		pHeader->Tracker = TrackerManager::RetrieveTrackerByName("BoxTracker");

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

	pStartMemBlock = pMem + sizeof(Header);
#else
	pStartMemBlock = pMem;
#endif

	BoxCreateLock.unlock();

	return pStartMemBlock;
}

void Box::operator delete(void* pMem)
{
	BoxCreateLock.lock();

	if (!pMem)
	{
		BoxCreateLock.unlock();
		return;
	}

#ifndef NDEBUG
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
		else if (pHeader->pNext == nullptr && pHeader->pPrev)
		{
			pHeader->pPrev->pNext = nullptr;
		}
		else if (pHeader->pNext && pHeader->pPrev == nullptr)
		{
			pHeader->pNext->pPrev = nullptr;
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

	BoxCreateLock.unlock();
	free(pHeader);
#else
	BoxCreateLock.unlock();
	free(pMem);
#endif
}

void Box::Draw()
{
	glPushMatrix();
	glTranslatef(Position.x, Position.y, Position.z);
	GLfloat diffuseMaterial[] = { Colour.x, Colour.y, Colour.z, 1.0f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glScalef(Size.x, Size.y, Size.z);
	glRotatef(-90, 1, 0, 0);
	glutSolidCube(1.0);
	glPopMatrix();
}

void Box::InitBox()
{
	// Assign random x, y, and z positions within specified ranges
	Position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
	Position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
	Position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

	Size = { 1.0f, 1.0f, 1.0f };

	// Assign random x-velocity between -1.0f and 1.0f
	float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
	Velocity = { randomXVelocity, 0.0f, 0.0f };

	// Assign a random color to the box
	Colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	Colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	Colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void Box::ResolveCollision(Box& OtherBox)
{
#ifndef NDEBUG
	auto start = std::chrono::steady_clock::now();
#endif

	Vec3 Normal = { Position.x - OtherBox.Position.x, Position.y - OtherBox.Position.y, Position.z - OtherBox.Position.z };
	float Length = Normal.Length();

	// Normalize the normal vector
	Normal.Normalise();

	float relativeVelocityX = Velocity.x - OtherBox.Velocity.x;
	float relativeVelocityY = Velocity.y - OtherBox.Velocity.y;
	float relativeVelocityZ = Velocity.z - OtherBox.Velocity.z;

	// Compute the relative velocity along the normal
	float impulse = relativeVelocityX * Normal.x + relativeVelocityY * Normal.y + relativeVelocityZ * Normal.z;

	// Ignore collision if objects are moving away from each other
	if (impulse > 0) 
	{
#ifndef NDEBUG
		std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
		TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
		return;
	}

	// Compute the collision impulse scalar
	float e = 0.01f; // Coefficient of restitution (0 = inelastic, 1 = elastic)
	float dampening = 0.9f; // Dampening factor (0.9 = 10% energy reduction)
	float j = -(1.0f + e) * impulse * dampening;

	// Apply the impulse to the boxes' velocities
	Velocity.x += j * Normal.x;
	Velocity.y += j * Normal.y;
	Velocity.z += j * Normal.z;
	OtherBox.Velocity.x -= j * Normal.x;
	OtherBox.Velocity.y -= j * Normal.y;
	OtherBox.Velocity.z -= j * Normal.z;

#ifndef NDEBUG
	std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
	TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

bool Box::CheckCollision(Box& OtherBox)
{
	return (std::abs(Position.x - OtherBox.Position.x) * 2 < (Size.x + OtherBox.Size.x)) &&
		(std::abs(Position.y - OtherBox.Position.y) * 2 < (Size.y + OtherBox.Size.y)) &&
		(std::abs(Position.z - OtherBox.Position.z) * 2 < (Size.z + OtherBox.Size.z));
}

void Box::Update(const float DeltaTime)
{
	// Update velocity due to gravity
	Velocity.y += Gravity * DeltaTime;

	// Update position based on velocity
	Position.x += Velocity.x * DeltaTime;
	Position.y += Velocity.y * DeltaTime;
	Position.z += Velocity.z * DeltaTime;

	// Check for collision with the floor
	if (Position.y - Size.y / 2.0f < FloorY) 
	{
		Position.y = FloorY + Size.y / 2.0f;
		float dampening = 0.7f;
		Velocity.y = -Velocity.y * dampening;
	}

	// Check for collision with the walls
	if (Position.x - Size.x / 2.0f < MinX || Position.x + Size.x / 2.0f > MaxX) {
		Velocity.x = -Velocity.x;
	}
	if (Position.z - Size.z / 2.0f < MinZ || Position.z + Size.z / 2.0f > MaxZ) {
		Velocity.z = -Velocity.z;
	}
}

bool Box::RayBoxIntersection(const Vec3& RayOrigin, const Vec3& RayDirection)
{
#ifndef NDEBUG
	auto start = std::chrono::steady_clock::now();
#endif

	float tMin = (Position.x - Size.x / 2.0f - RayOrigin.x) / RayDirection.x;
	float tMax = (Position.x + Size.x / 2.0f - RayOrigin.x) / RayDirection.x;

	if (tMin > tMax) std::swap(tMin, tMax);

	float tyMin = (Position.y - Size.y / 2.0f - RayOrigin.y) / RayDirection.y;
	float tyMax = (Position.y + Size.y / 2.0f - RayOrigin.y) / RayDirection.y;

	if (tyMin > tyMax) std::swap(tyMin, tyMax);

	if ((tMin > tyMax) || (tyMin > tMax))
	{
#ifndef NDEBUG
		std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
		TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
		return false;
	}

	if (tyMin > tMin)
		tMin = tyMin;

	if (tyMax < tMax)
		tMax = tyMax;

	float tzMin = (Position.z - Size.z / 2.0f - RayOrigin.z) / RayDirection.z;
	float tzMax = (Position.z + Size.z / 2.0f - RayOrigin.z) / RayDirection.z;

	if (tzMin > tzMax) std::swap(tzMin, tzMax);

	if ((tMin > tzMax) || (tzMin > tMax))
	{
#ifndef NDEBUG
		std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
		TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
		return false;
	}

#ifndef NDEBUG
	std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
	TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif

	return true;
}