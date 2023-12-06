// TODO 
// Tidy and label the code
// add memory chunks to the box
// C++ it all
// convert to PS4? (or make this a student task?) - check out the PS4 SDK samples
// rename project etc. 

#ifndef NDEBUG
#include "ProgramProfiler.h"
#include "TimeProfiler.h"
#endif

#include "Constants.h"
#include "Box.h"
#include "QuadTree.h"

QuadTree* Trunk = nullptr;

void initScene(int boxCount) {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif
    Trunk = new QuadTree(TREE_DEPTH, true);

    for (int i = 0; i < boxCount; ++i) {
        Box* box = new Box();

        box->InitBox();

        Trunk->AddInitialisedBox(box);
    }

    Trunk->SetBoxesInitialised(true);

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// used in the 'mouse' tap function to convert a screen point to a point in the world
Vec3 screenToWorld(int x, int y) {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif

    return Vec3((float)posX, (float)posY, (float)posZ);
}

// update the physics: gravity, collision test, collision resolution
void updatePhysics(const float deltaTime) {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    Trunk->Update(deltaTime);

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// draw the sides of the containing area
void drawQuad(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4) {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    glBegin(GL_QUADS);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glVertex3f(v4.x, v4.y, v4.z);
    glEnd();

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// draw the entire scene
void drawScene() {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    // Draw the side wall
    GLfloat diffuseMaterial[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);

    // Draw the left side wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 leftSideWallV1(MinX, 0.0f, MaxZ);
    Vec3 leftSideWallV2(MinX, 50.0f, MaxZ);
    Vec3 leftSideWallV3(MinX, 50.0f, MinZ);
    Vec3 leftSideWallV4(MinX, 0.0f, MinZ);
    drawQuad(leftSideWallV1, leftSideWallV2, leftSideWallV3, leftSideWallV4);

    // Draw the right side wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 rightSideWallV1(MaxX, 0.0f, MaxZ);
    Vec3 rightSideWallV2(MaxX, 50.0f, MaxZ);
    Vec3 rightSideWallV3(MaxX, 50.0f, MinZ);
    Vec3 rightSideWallV4(MaxX, 0.0f, MinZ);
    drawQuad(rightSideWallV1, rightSideWallV2, rightSideWallV3, rightSideWallV4);

    // Draw the back wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 backWallV1(MinX, 0.0f, MinZ);
    Vec3 backWallV2(MinX, 50.0f, MinZ);
    Vec3 backWallV3(MaxX, 50.0f, MinZ);
    Vec3 backWallV4(MaxX, 0.0f, MinZ);
    drawQuad(backWallV1, backWallV2, backWallV3, backWallV4);

    Trunk->Draw();

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// called by GLUT - displays the scene
void display() {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(LOOKAT_X, LOOKAT_Y, LOOKAT_Z, LOOKDIR_X, LOOKDIR_Y, LOOKDIR_Z, 0, 1, 0);

    drawScene();

    glutSwapBuffers();

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// called by GLUT when the cpu is idle - has a timer function you can use for FPS, and updates the physics
// see https://www.opengl.org/resources/libraries/glut/spec3/node63.html#:~:text=glutIdleFunc
// NOTE this may be capped at 60 fps as we are using glutPostRedisplay(). If you want it to go higher than this, maybe a thread will help here. 
void idle() {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    static auto last = steady_clock::now();
    auto old = last;
    last = steady_clock::now();
    const duration<float> frameTime = last - old;
    float deltaTime = frameTime.count();

    updatePhysics(deltaTime);

    // tell glut to draw - note this will cap this function at 60 fps
    glutPostRedisplay();

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// called the mouse button is tapped
void mouse(int button, int state, int x, int y) {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Get the camera position and direction
        Vec3 cameraPosition(LOOKAT_X, LOOKAT_Y, LOOKAT_Z); // Replace with your actual camera position
        Vec3 cameraDirection(LOOKDIR_X, LOOKDIR_Y, LOOKDIR_Z); // Replace with your actual camera direction

        // Get the world coordinates of the clicked point
        Vec3 clickedWorldPos = screenToWorld(x, y);

        // Calculate the ray direction from the camera position to the clicked point
        Vec3 rayDirection = clickedWorldPos - cameraPosition;
        rayDirection.Normalise();

        // Perform a ray-box intersection test and remove the clicked box
        size_t clickedBoxIndex = -1;
        float minIntersectionDistance = std::numeric_limits<float>::max();

        for (size_t i = 0; i < Trunk->Boxes.size(); ++i) {
            if (Trunk->Boxes[i]->RayBoxIntersection(cameraPosition, rayDirection)) {
                // Calculate the distance between the camera and the intersected box
                Vec3 diff = Trunk->Boxes[i]->Position - cameraPosition;
                float distance = diff.Length();

                // Update the clicked box index if this box is closer to the camera
                if (distance < minIntersectionDistance) {
                    clickedBoxIndex = i;
                    minIntersectionDistance = distance;
                }
            }
        }

        // Remove the clicked box if any
        if (clickedBoxIndex != -1) {
            Trunk->Boxes.erase(Trunk->Boxes.begin() + clickedBoxIndex);
        }
    }

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// called when the keyboard is used
void keyboard(unsigned char key, int x, int y) {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    const float impulseMagnitude = 20.0f; // Upward impulse magnitude

    if (key == ' ') { // Spacebar key
        for (Box* box : Trunk->Boxes) {
            box->Velocity.y += impulseMagnitude;
        }
    }

#ifndef NDEBUG
    if (key == '1')
    {
        TrackerManager::WalkTheHeap();
    }
#endif

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif
}

// the main function. 
int main(int argc, char** argv) {
#ifndef NDEBUG
    auto start = steady_clock::now();
#endif

    srand(static_cast<unsigned>(time(0))); // Seed random number generator
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Simple Physics Simulation");

    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    //Trunk = new QuadTree(TREE_DEPTH, true);

    initScene(NUMBER_OF_BOXES);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

#ifndef NDEBUG
    duration<float> diff = steady_clock::now() - start;
    TimeProfiler::AddData(__FUNCTION__, diff.count());
#endif

    // it will stick here until the program ends. 
    glutMainLoop();

    Trunk->SetProgramRunning(false);

#ifndef NDEBUG
    vector<TimeData> Temp = TimeProfiler::TimeProfilerData;
    for (int i = 0; i < Temp.size(); i++)
    {
        std::cout << "Function Name: " << Temp[i].FunctionName << std::endl; 
        std::cout << "  -Total Time Taken: " << Temp[i].TotalTime << "s. \n";
        std::cout << "  -Average Time: " << Temp[i].AverageTime << " s. \n";
        std::cout << "  -Times Called: " << Temp[i].TimesCalled << "\n" << "\n";
    }
#endif

    return 0;
}
