#include <iostream>
#include <stdexcept>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    MessageBox(nullptr, "Hello world!", "Retro-3D-Raycasting-Game", MB_OK);

    return 0;
}