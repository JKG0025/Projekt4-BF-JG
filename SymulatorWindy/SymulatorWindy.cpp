// SymulatorWindy.cpp : Defines the entry point for the application.
//

#include "SymulatorWindy.h"
#include "GUI.h"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE /*hPrevInst*/, PWSTR /*pszCmdLine*/, int nCmdShow)
{
	// tu masz już w hInst uchwyt do modułu
	GdiplusWindow win(hInst, L"Moje Okno", 800, 600, L"tlo.jpg");

	return win.RunMessageLoop();
}

