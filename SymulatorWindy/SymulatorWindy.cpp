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
   for (int i = 0; i < 5; i++)
   {
	   if (i == 0) i++;
       window->AddButton(
           std::to_wstring(i), 0, 518 - i * 25, 25, 25,
           [this, i]() 
		   {
               onButtonClick(0, i, 253 - 23 * elevatorLogic.passengerCount(0), 472);
           }
       );
   }

   for (int i = 0; i < 5; i++)
   {
	   if (i == 1) i++;
	   window->AddButton(
		   std::to_wstring(i), 760, 418 - i * 25, 25, 25,
		   [this, i]()
		   {
			   onButtonClick(1, i, 500 + 23 * elevatorLogic.passengerCount(1), 372);
		   }
	   );
   }

   for (int i = 0; i < 5; i++) //help malo elegancko te przyciski wyszly
   {
	   if (i == 2) i++;
	   window->AddButton(
		   std::to_wstring(i), 0, 338 - i * 25, 25, 25,
		   [this, i]()
		   {
			   onButtonClick(2, i, 253 - 23 * elevatorLogic.passengerCount(2), 292);
		   }
	   );
   }

   for (int i = 0; i < 5; i++)
   {
	   if (i == 3) i++;
	   window->AddButton(
		   std::to_wstring(i), 760, 194 - i * 25, 25, 25,
		   [this, i]()
		   {
			   onButtonClick(3, i, 500 + 23 * elevatorLogic.passengerCount(3), 148);
		   }
	   );
   }

   for (int i = 0; i < 5; i++)
   {
	   if (i == 4) break; // No button for the 5th floor, as it is the top floor
	   window->AddButton(
		   std::to_wstring(i), 0, 154 - i * 25, 25, 25,
		   [this, i]()
		   {
			   onButtonClick(4, i, 253 - 23 * elevatorLogic.passengerCount(4), 108);
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
