// SymulatorWindy.cpp : Defines the entry point for the application.
//

#include "SymulatorWindy.h"
#include "GUI.h"

int main()
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    // dalej jak w WinMain:
    GdiplusWindow win(hInst, L"Moje Okno", 600, 450, L"zdjencia\\szybwindy.png");
    win.Show();
    return win.RunMessageLoop();
}

