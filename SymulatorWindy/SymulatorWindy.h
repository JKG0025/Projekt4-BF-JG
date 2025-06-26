// SymulatorWindy.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <Windows.h>
#include <iostream>
#include <gdiplus.h>
#include "ElevatorLogic.h"

class elevatorWindow
{
public:
	elevatorWindow(GdiplusWindow& window_);
	int runMessageLoop();

private:
	GdiplusWindow* window;
	ElevatorLogic* elevatorLogic;
	void onButtonClick(int initialFloor, int destination, int x, int y);
};

