#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <math.h>

typedef struct {
    /*
    * Structure for entity data.
    */
    float x;
    float y;
    float z;
    float pitch;
    float yaw;
    int hp;
} Entity;

DWORD GetProcessIdByName(char *ProcessName) {

    /*
    * Function: GetProcessIdByName
    * -------------------------------------------------------
    * Gets the process ID by comparing all processes names with desired.
    * 
    * Return: (unsigned int) ID of the process,
    *         returns 0 if process doesn't exist.
    */

    DWORD ProcessId = 0;
    /*
    * ProcessInfo is the struct where
    * information about process stored.
    */
    PROCESSENTRY32 ProcessInfo;
    /*
    * It's important to set dwSize to sizeof PE32,
    * if you don't, Process32...() funtions fail.
    */
    ProcessInfo.dwSize = sizeof(PROCESSENTRY32);
    /*
    * Creating snapshot of all processes
    */
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    /*
    * 
    */
    while ( Process32Next(hSnapshot, &ProcessInfo) ) {
        if ( strcmp(ProcessInfo.szExeFile, ProcessName) == 0 ) {
            ProcessId = ProcessInfo.th32ProcessID;
            break;
        }
    }
    return ProcessId;
}

void RotateAroundAxis(Entity *point, char Axis, float angle) {

    /*
    * Function: RotateAroundAxis
    * -------------------------------------------------------
    * Calculates 3D coordinates of a point
    * after counterclock-wise rotation
    * around axis by specified angle
    * using 4x4 matrices.
    * 
    * Returns: Writes X, Y, Z coordinates
    *          into Entity struct.
    */
    /*
    * Return if axis inavalid.
    */
    if (Axis != 'X' && Axis != 'Y' && Axis != 'Z') return;
    /*
    * Constant amount of radians in 1 degree.
    * PI / 180
    */
    const float RADIAN = 0.0174532925;
    /*
    * Constant sine and cosine.
    */
    const float SINE = sin(angle * RADIAN);
    const float COSINE = cos(angle * RADIAN);
    /*
    * Rotation around X axis matrix:
    * 
    *   1     0     0     0
    * 
    *   0    cos   sin    0
    *
    *   0   -sin   cos    0
    *
    *   0     0     0     1
    * 
    * Multiplying [ x y z w ] matrix by rotation matrix gives:
    * 
    * x = x * 1
    * 
    * y = y * cos + z * -sin
    * 
    * z = y * sin + z * cos
    * 
    * w = w * 1
    */
    if (Axis == 'X') {

        /*
        * Storing values in another variables
        * so they don't interfere multiplication.
        * 
        * When rotating around an axis,
        * this axis coordinate is constant,
        * so it's unnecessary to return it.
        * 
        * This comment is valid for all axes below.
        */
        float y = point->y * COSINE + point->z * -SINE;
        float z = point->y * SINE + point->z * COSINE;

        point->y = y;
        point->z = z;

        return;
    }
    /*
    * Rotation around Y axis matrix:
    * 
    *  cos    0   -sin    0
    * 
    *   0     1     0     0
    *
    *  sin    0    cos    0
    *
    *   0     0     0     1
    * 
    * Multiplying [ x y z w ] matrix by rotation matrix gives:
    * 
    * x = x * cos + z * sin
    * 
    * y = y * 1
    * 
    * z = x * -sin + z * cos
    * 
    * w = w * 1
    */
    if (Axis == 'Y') {

        float x = point->x * COSINE + point->z * SINE;
        float z = point->x * -SINE + point->z * COSINE;

        point->x = x;
        point->z = z;

        return;
    }
    /*
    * Rotation around Z axis matrix:
    *
    *  cos   sin    0     0
    *
    * -sin   cos    0     0
    *
    *   0     0     1     0
    *
    *   0     0     0     1
    * 
    * Multiplying [ x y z w ] matrix by rotation matrix gives:
    * 
    * x = x * cos + y * -sin
    * 
    * y = x * sin + y * cos
    * 
    * z = z * 1
    * 
    * w = w * 1
    */
    if (Axis == 'Z') {

        float x = point->x * COSINE + point->y * -SINE;
        float y = point->x * SINE + point->y * COSINE;

        point->x = x;
        point->y = y;

        return;
    }

}

void CameraTransform(Entity *point, float Y_Fov) {

    /*
    * Function: CameraTransform
    * -------------------------------------------------------
    * Translates point from camera space
    * to viewing space using 4x4 matrix.
    * 
    * Returns: Writes X, Y, Z coordinates
    *          into Entity struct.
    */
    /*
    * Constant amount of radians in 1 degree.
    * PI / 180
    */
    const float RADIAN = 0.0174532925;
    /*
    * Constant cotangent of X axis FOV 
    * which is 45 degrees.
    */
    const float CTG_X = 1;
    /*
    * Constant cotangent of Y axis FOV.
    */
    const float CTG_Y = 1 / tan(Y_Fov * RADIAN);
    /*
    * Camera transform matrix:
    * 
    *  ctgx   0     0     0
    * 
    *   0    ctgy   0     0
    *
    *   0     0     a    -1
    *
    *   0     0     b     0
    * 
    * Where a = (f + n) / (f - n),
    *       b = (2 * f * n) / (f - n),
    *       where n = 1 is near and f = 1000 is far.
    * 
    * Approx. a = 1; b = 2;
    * 
    * Multiplying [ x y z w ] matrix by translation matrix gives:
    * 
    * x = x * CTG_X
    * 
    * y = y * CTG_Y
    * 
    * z = z * a + w * b
    * 
    * w = z * -1
    * 
    * Now, the w value is not same for every point, 
    * to complete the transformation we need to divide
    * every point by w.
    * Because w is actually -z we can replace it.
    * 
    * Finally it gives us:
    * 
    * x = x * CTG_X / -z
    * 
    * y = y * CTG_Y / -z
    * 
    * z = (z + 2) / -z
    * 
    * w = 1
    */
    point->x = point->x * CTG_X / -point->z;
    point->y = point->y * CTG_Y / -point->z;
    
    /*
    * ======================= THEORY HERE ========================
    *
    * With rotation functions we get the position of the Enemy
    * from the Player's point of view, where Player look
    * vector matches -z axis and x axis is to the Player right.
    * 
    *                          y  ^
    *                             |         ⟋|
    *                             |       ⟋ Example point (0, 2, -3)
    *                             |     ⟋  / |
    *                             |   ⟋   *  |
    *   z                         | ⟋|       |
    *   <-------------------------⟋  |       |------------------
    *                           x |⟍ |       |
    *                             |  ⟍       |
    *                             |  n ⟍     |
    *                             |      ⟍   |
    *                             |        ⟍ |
    *                             |          f
    * 
    *               (X axis is directed towards you)
    * 
    * This is the camera object facing at the -z direction,
    * shown triangle is YZ view of the camera pyramid.
    * Near (n) and far (f) lines limit the camera view shape
    * turning it into a truncated pyramid.
    * 
    * Camera tranform function transforms truncated pyramid into a cube.
    * Objects near the camera become bigger and objects far from the camera become smaller.
    *
    *                         y  ^
    *                            | 1
    *                   _________|_________
    *                  |         | Example point (0, 0.5, 0.6667)
    *                  |         *         |
    *               -1 |         |         | 1                 x
    *   ---------------|---------|---------|------------------->
    *                  |       z |         |
    *                  |         |         |
    *                  |_________|_________|
    *                            | -1
    *                            |
    *                            |
    * 
    *           (now Z axis is directed towards you)
    * 
    * This cube called the perspective projection, 
    * shown square is the XY view of the cube, in other words 2x2 square screen
    * Now we can throw the z out and map the point on the plane,
    * resize it to the window resolution and we're done.
    * 
    * More info can be found here:
    * http://www.codinglabs.net/article_world_view_projection_matrix.aspx
    */  
}

void WorldToScreen(Entity *Player, Entity *Enemy, float Y_Fov,
                   float WindowX, float WindowY) {

    /*
    * Function: WorldToScreen
    * -------------------------------------------------------
    * Translates 3D point from game world to player screen.
    * 
    * Returns: Writes X and Y pixel coordinates
    * into Enemy Entity struct.
    */
    /*
    * Find position of the Enemy relative to the Player
    * to make Player the origin of camera coordinate system.
    */
    Enemy->x = Enemy->x - Player->x;
    Enemy->y = Enemy->y - Player->y;
    Enemy->z = Enemy->z - Player->z;
    /*
    * Simulate camera rotation
    * by rotating point around static camera (origin)
    * in the opposite direction.
    */
    RotateAroundAxis(Enemy, 'Y', Player->yaw);
    RotateAroundAxis(Enemy, 'X', -Player->pitch);

    if ( Enemy->z >= 0 ) {
        /*
        * Camera is facing in -z direction, if Enemy z
        * is more than 0 that means Enemy is behind camera object,
        * Set x to -1, telling that enemy is offscreen.
        */
        Enemy->x = -1;
        return;
    }
    /*
    * Performing camera transform on Enemy point.
    */
    CameraTransform(Enemy, Y_Fov / 2);
    /*
    * If Enemy point is off perspective projection square
    * then the Enemy is offscreen.
    */
    if (Enemy->x >= -1 && Enemy->x <= 1 && Enemy->y >= -1 && Enemy->y <= 1) { 
        Enemy->x = (Enemy->x + 1) * WindowX / 2;
        Enemy->y = WindowY - ((Enemy->y + 1) * WindowY / 2);
    }

    else {
        Enemy->x = -1;
    }

}

int main(int argc, char *argv[]) {
    /*
    * Initialize Player variables.
    */
    Entity Player;
    float Y_Fov;
    /*
    * Initialize Enemy variables.
    */
    Entity Enemy;
    int EntityCount;
    /*
    * Initialize other variables.
    */
    int WindowX;
    int WindowY;
    int BoxSize;
    /*
    * Player entity offset.
    */
    uintptr_t PlayerEntPtr = 0x57E0A8;
    uintptr_t PlayerEntAddr = 0;
    /*
    * Entity List offset.
    */
    uintptr_t EntityListPtr = 0x58AC04;
    uintptr_t EntityListAddr = 0;

    uintptr_t EntityAddr = 0;
    /*
    * Entity count offset.
    */
    uintptr_t EntityCountPtr = 0x45C434;
    uintptr_t EntityCountAddr = 0;
    /*
    * Y axis FOV address.
    */
    uintptr_t FovYAddr = 0x57E0A4;
    /*
    * Window size offset,
    * Y is 4 bytes away.
    */
    uintptr_t WindowXAddr = 0x591ED8;
    /*
    * Getting AssaultCube ProcessId.
    */
    DWORD ProcessId = GetProcessIdByName("ac_client.exe");
    /*
    * Getting handle to AssaultCube window.
    */
    HWND hWindow = FindWindowA(0, "AssaultCube");
    /*
    * Getting handle to device context,
    * used to draw boxes inside the window.
    */
    HDC ACDraw = GetDC(hWindow);

    RECT Box;
    HBRUSH Brush = CreateSolidBrush(RGB(0, 255, 0));
    /*
    * Initializing handle with ac_client.exe process.
    */
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
    /*
    * Getting address of local Player.
    */
    ReadProcessMemory(hProcess, (LPCVOID) PlayerEntPtr, &PlayerEntAddr, 4, NULL);
    /*
    * Getting address of entity list.
    */
    ReadProcessMemory(hProcess, (LPCVOID) EntityListPtr, &EntityListAddr, 4, NULL);
    /*
    * Initializing pointer to iterate over entity list.
    */
    uintptr_t EntityPtrAddr = EntityListAddr;

    while ( TRUE ) {
        /*
        * Getting address of Enemy count.
        */
        ReadProcessMemory(hProcess, (LPCVOID) EntityCountPtr, &EntityCountAddr, 4, NULL);
        /*
        * Getting Enemy count
        */
        ReadProcessMemory(hProcess, (LPCVOID) EntityCountAddr, &EntityCount, 4, NULL);
        /*
        * If there are Enemies.
        */
        if (EntityCount > 1) {
            /*
            * Getting Player info.
            */
            ReadProcessMemory(hProcess, (LPCVOID) PlayerEntAddr + 0x4, &Player.x, 4, NULL);
            ReadProcessMemory(hProcess, (LPCVOID) PlayerEntAddr + 0x8, &Player.z, 4, NULL);
            ReadProcessMemory(hProcess, (LPCVOID) PlayerEntAddr + 0xC, &Player.y, 4, NULL);

            ReadProcessMemory(hProcess, (LPCVOID) PlayerEntAddr + 0x38, &Player.pitch, 4, NULL);
            ReadProcessMemory(hProcess, (LPCVOID) PlayerEntAddr + 0x34, &Player.yaw, 4, NULL);

            ReadProcessMemory(hProcess, (LPCVOID) FovYAddr, &Y_Fov, 4, NULL);
            /*
            * Getting window info.
            */
            ReadProcessMemory(hProcess, (LPCVOID) WindowXAddr, &WindowX, 4, NULL);
            ReadProcessMemory(hProcess, (LPCVOID) WindowXAddr + 0x4, &WindowY, 4, NULL);
            /*
            * Iterating over entity list.
            */
            for ( int i = 0; i < EntityCount - 1; i++ ) {

                EntityPtrAddr += 0x4;
                /*
                * Getting address of Enemy.
                */
                ReadProcessMemory(hProcess, (LPCVOID) EntityPtrAddr, &EntityAddr, 4, NULL);
                /*
                * Getting Enemy health for check.
                */
                ReadProcessMemory(hProcess, (LPCVOID) EntityAddr + 0xEC, &Enemy.hp, 4, NULL);
                /*
                * If Enemy is alive.
                */
                if ( Enemy.hp > 0 ) {
                    /*
                    * Getting Enemy position
                    */
                    ReadProcessMemory(hProcess, (LPCVOID) EntityAddr + 0x4, &Enemy.x, 4, NULL);
                    ReadProcessMemory(hProcess, (LPCVOID) EntityAddr + 0x8, &Enemy.z, 4, NULL);
                    ReadProcessMemory(hProcess, (LPCVOID) EntityAddr + 0xC, &Enemy.y, 4, NULL);
                    Enemy.y += 0.2;
                    Enemy.x += 0.2;
                    /*
                    * Running w2s function.
                    */
                    WorldToScreen(&Player, &Enemy, Y_Fov, WindowX, WindowY);
                    /*
                    * Calculating box position and size.
                    */
                    BoxSize = 1 / -Enemy.z * 750;
                    Box.bottom = (int) Enemy.y + BoxSize;
                    Box.top = (int) Enemy.y - BoxSize;
                    Box.left = (int) Enemy.x -BoxSize;
                    Box.right = (int) Enemy.x + BoxSize;
                    /*
                    * If Enemy on screen.
                    */
                    if (Enemy.x >= 0) FrameRect(ACDraw, &Box, Brush);
                }
            }

            EntityPtrAddr = EntityListAddr;
        }
    }

    return 0;
}