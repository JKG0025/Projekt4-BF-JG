// SymulatorWindy.cpp : Defines the entry point for the application.
//

#include "SymulatorWindy.h"


int main()
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	GdiplusWindow temp(hInst, L"Symulator windy", 800, 600, L".\\zdjencia\\sybwindy.png");
	elevatorWindow win(temp);

	return win.runMessageLoop();
}

elevatorWindow::elevatorWindow(GdiplusWindow& window_)
{
   passengersOnFloors.resize(5); // Initialize 5 floors
   window = &window_;
   for (int i = 0; i < 4; i++)
   {
       window->AddButton(
           std::to_wstring(i + 1), 0, 518 - i * 25, 25, 25,
           [this, i]() 
		   {
               onButtonClick(0, i + 1, 50 + 23 * elevatorLogic.passengerCount(0), 473);
           }
       );
   }
   window->Show(SW_SHOW);
}

int elevatorWindow::runMessageLoop()
{
	while (true)
	{
		int currentFloor = elevatorLogic.elevatorLoop(time(nullptr));
		window->MoveSprite(0, 15 + 23 * currentFloor, 518); // Move elevator sprite to the current floor position
		if (window->RunMessageLoop() == -1) // Exit if message loop returns -1
			break;
	}
	return 0;
}

void elevatorWindow::onButtonClick(int initialFloor, int destination, int x, int y)
{
	elevatorLogic.addPassenger(initialFloor, destination);
	window->AddSprite(L".\\zdjencia\\" + std::to_wstring(destination) + L"ludzikbasic.png", x, y);

}
