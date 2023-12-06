#pragma once

#include <ctime>
#include <chrono>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;
using namespace chrono;

#ifndef CONSTANTS_H
#define CONSTANTS_H

// this is the number of falling physical items. 
#define NUMBER_OF_BOXES 10000

#define NUMBER_OF_THREADS 32

#define MEMORY_POOL_SIZE 100

#define VISUALISE_TREE 1

//define tree depth for quadtree
#define TREE_DEPTH 3

// these is where the camera is, where it is looking and the bounds of the continaing box. You shouldn't need to alter these
#define LOOKAT_X 10
#define LOOKAT_Y 30
#define LOOKAT_Z 50

#define LOOKDIR_X 10
#define LOOKDIR_Y 0
#define LOOKDIR_Z 0

#define MinX -10.0f
#define MaxX 30.0f
#define MinZ -30.0f
#define MaxZ 30.0f

#define Gravity -19.81f

#define FloorY 0.0f

#endif