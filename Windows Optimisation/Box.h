#pragma once
#ifndef BOX_H
#define BOX_H

#include <cmath>

class Vec3
{
public:
	float x, y, z;

	Vec3() : x(0.0f), y(0.0f), z(0.0f) {};
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {};

	inline Vec3 operator-(const Vec3& other) const
	{
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	inline float Length() const { return std::sqrt(x * x + y * y + z * z); }

	void Normalise();
};

class Box
{
public:
	Vec3 Position;
	Vec3 Size;
	Vec3 Velocity;
	Vec3 Colour;

	void InitBox();
	void ResolveCollision(Box& OtherBox);
	bool CheckCollision(Box& OtherBox);
	void Update(const float DeltaTime);
	void Draw();
	bool RayBoxIntersection(const Vec3& RayOrigin, const Vec3& RayDirection);


	void* operator new(size_t size);
	void operator delete(void* pMem);
};

#endif