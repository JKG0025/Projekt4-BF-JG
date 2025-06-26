#include "ElevatorLogic.h"

ElevatorLogic::ElevatorLogic(GdiplusWindow* window_) : window(window_), currentFloor(0), goingUp(false)
{
	// Initialize the floorPassengers vector with 5 floors
	floorPassengers.resize(5);
	for (int i = 0; i < 5; i++)
	{
		floorPassengers[i].clear(); // Ensure each floor starts with no passengers
	}
	elevatorData = new elevator(window->AddSprite(L".\\zdjencia\\winda.png", ELEVATOR_START_X, FLOOR_EXITS[0].Y + ELEVATOR_Y_OFFSET));
	textId = window->AddText(L"Waga pasa¿erów: 0kg", textPosition.X, textPosition.Y, L"Arial", 16, Gdiplus::Color(255, 0, 0, 0));
}

bool ElevatorLogic::elevatorLoop(time_t timeSinceStop, bool wasEmpty)
{
	// 1. Unload passengers whose destination is the current floor
	unloadPassengersAtCurrentFloor();

	// 2. Load passengers from the current floor
	loadPassengersAtCurrentFloor();

	// 3. Decide elevator direction
	wasEmpty = updateDirection(timeSinceStop, wasEmpty);

	// 4. Animate all passengers in the elevator to their new positions
	animatePassengersInElevator();

	// 5. Move the elevator sprite to the current floor
	moveElevatorSprite();

	return wasEmpty;
}
void ElevatorLogic::unloadPassengersAtCurrentFloor()
{
	std::vector<passenger*> leavingPassengers;

	// 1. Mark passengers to leave and animate them directly to off-screen position
	for (int i = static_cast<int>(passengersInElevator.size()) - 1; i >= 0; --i)
	{
		auto* p = passengersInElevator[i];
		if (p->destination == currentFloor)
		{
			p->isInElevator = false;
			leavingPassengers.push_back(p);
			// Animate directly to off-screen position
			int offscreenX = (currentFloor % 2 == 0) ? (FLOOR_EXITS[currentFloor].X - 200) : (FLOOR_EXITS[currentFloor].X + 200);
			window->AnimateSprite(p->passengerId,
				offscreenX,
				FLOOR_EXITS[currentFloor].Y,
				ANIMATION_SPEED_PX_PER_SEC, true); // true: delete after animation
			passengersInElevator.erase(passengersInElevator.begin() + i);
			window->EditText(textId, L"Waga pasa¿erów: " + std::to_wstring(passengersInElevator.size() * 70) + L"kg", textPosition.X, textPosition.Y, L"Arial", 16, Gdiplus::Color(255, 0, 0, 0));
		}
	}

	// 2. Wait for all leaving passengers to cross the floor exit (just outside elevator)
	for (auto* p : leavingPassengers)
	{
		// Wait until the sprite's X position is past the floor exit (just outside elevator)
		int exitX = (currentFloor % 2 == 0) ? 253 : 500;
		// Wait until the sprite's X is less than exitX (left) or greater than exitX (right)
		while (true)
		{
			int spriteX = window->getSpriteX(p->passengerId);
			if ((currentFloor % 2 == 0 && spriteX <= exitX) ||
				(currentFloor % 2 != 0 && spriteX >= exitX))
			{
				break;
			}
			window->WaitForDuration(10); // Small delay to avoid busy waiting
		}
	}

	// 3. Reposition remaining elevator passengers
	for (size_t i = 0; i < passengersInElevator.size(); ++i)
	{
		window->AnimateSprite(passengersInElevator[i]->passengerId,
			ELEVATOR_START_X + SPACING * static_cast<int>(i),
			FLOOR_EXITS[currentFloor].Y,
			ANIMATION_SPEED_PX_PER_SEC, false);
	}

	// 4. Wait for the last animation of repositioning (if any)
	if (!passengersInElevator.empty())
	{
		window->WaitForSpriteAnimation(passengersInElevator.back()->passengerId);
	}
}

void ElevatorLogic::loadPassengersAtCurrentFloor()
{
	auto& queue = floorPassengers[currentFloor];
	bool upward = goingUp;
	size_t idx = 0;
	std::vector<passenger*> loadedThisTurn;
	while (idx < queue.size() && passengersInElevator.size() < MAX_CAPACITY)
	{
		auto* p = queue[idx];
		bool wantsToGo = upward ? (p->destination > currentFloor)
			: (p->destination < currentFloor);
		if (wantsToGo)
		{
			window->AnimateSprite(p->passengerId,
				ELEVATOR_START_X + SPACING * static_cast<int>(passengersInElevator.size()),
				FLOOR_EXITS[currentFloor].Y,
				ANIMATION_SPEED_PX_PER_SEC, false);
			loadedThisTurn.push_back(p);
			p->isInElevator = true;
			passengersInElevator.push_back(p);
			window->EditText(textId, L"Waga pasa¿erów: " + std::to_wstring(passengersInElevator.size() * 70) + L"kg", textPosition.X, textPosition.Y, L"Arial", 16, Gdiplus::Color(255, 0, 0, 0));
			queue.erase(queue.begin() + idx);
		}
		else
		{
			++idx;
		}
	}
	// Wait only for the last passenger loaded this turn, if any
	if (!loadedThisTurn.empty())
	{
		window->WaitForSpriteAnimation(loadedThisTurn.back()->passengerId);
	}
	repositionFloorQueue(queue);
}

void ElevatorLogic::repositionFloorQueue(const std::vector<passenger*>& queue)
{
	for (size_t i = 0; i < queue.size(); ++i)
	{
		int offset = OFFSET_BASE * static_cast<int>(i);
		int x = (currentFloor % 2 == 0) ? (LEFT_X - offset) : (RIGHT_X + offset);
		window->AnimateSprite(queue[i]->passengerId,
			x,
			FLOOR_EXITS[currentFloor].Y,
			ANIMATION_SPEED_PX_PER_SEC, false);
	}
}

bool ElevatorLogic::updateDirection(time_t timeSinceStop, bool wasEmpty)
{
	bool hasAbove = isDestinationAbove(currentFloor);
	bool hasBelow = isDestinationBelow(currentFloor);
	bool empty = passengersInElevator.empty();

	if (goingUp)
	{
		if (hasAbove)
		{
			wasEmpty = false;
			++currentFloor;
		}
		else if (hasBelow)
		{
			wasEmpty = false;
			goingUp = false;
			--currentFloor;
		}
		else if (empty)
		{
			wasEmpty = true;
			handleIdleBehavior(timeSinceStop);
		}
	}
	else // going down
	{
		if (hasBelow)
		{
			wasEmpty = false;
			--currentFloor;
		}
		else if (hasAbove)
		{
			wasEmpty = false;
			goingUp = true;
			++currentFloor;
		}
		else if (empty)
		{
			wasEmpty = true;
			handleIdleBehavior(timeSinceStop);
		}
	}
	if (currentFloor == 0)
	{
		goingUp = true; // Always go up from ground floor
	}
	else if (currentFloor == 4)
	{
		goingUp = false; // Always go down from top floor
	}
	return wasEmpty;
}

void ElevatorLogic::handleIdleBehavior(time_t timeSinceStop)
{
	if (timeSinceStop >= IDLE_THRESHOLD && currentFloor > 0)
	{
		// Return to ground floor after idle time
		goingUp = false;
		--currentFloor;
	}
	else if (timeSinceStop < IDLE_THRESHOLD)
	{
		// Briefly reverse direction to look for calls
		goingUp = !goingUp;
	}
}

void ElevatorLogic::animatePassengersInElevator()
{
	for (size_t i = 0; i < passengersInElevator.size(); ++i)
	{
		window->AnimateSprite(passengersInElevator[i]->passengerId,
			ELEVATOR_START_X + SPACING * static_cast<int>(i),
			FLOOR_EXITS[currentFloor].Y,
			ANIMATION_SPEED_PX_PER_SEC, false);
	}
}

void ElevatorLogic::moveElevatorSprite()
{
	window->AnimateSprite(elevatorData->elevatorId,
		ELEVATOR_START_X,
		FLOOR_EXITS[currentFloor].Y + ELEVATOR_Y_OFFSET,
		ANIMATION_SPEED_PX_PER_SEC, false);
	window->WaitForSpriteAnimation(elevatorData->elevatorId);
}



void ElevatorLogic::addPassenger(int startFloor, int destination, size_t spriteId)
{
	if (startFloor < 0 || startFloor > 4 || destination < 0 || destination > 4 || startFloor == destination)
	{
		exit(EXIT_FAILURE); // Invalid floor or destination
	}
	floorPassengers[startFloor].push_back(new passenger(startFloor, destination, false, spriteId));
}

bool ElevatorLogic::isDestinationAbove(int floor)
{
	for (int i = floorPassengers.size() - 1; i > floor; i--)
	{
		if (!floorPassengers[i].empty() && passengersInElevator.size() < 7)
		{
			return true;
		}
	}
	for (int i = 0; i < passengersInElevator.size(); i++)
	{
		if (passengersInElevator[i]->destination > floor)
		{
			return true;
		}
	}
	return false;
}

bool ElevatorLogic::isDestinationBelow(int floor)
{
	for (int i = 0; i < floor; i++)
	{
		if (!floorPassengers[i].empty() && passengersInElevator.size() < 7)
		{
			return true;
		}
	}
	for (int i = 0; i < passengersInElevator.size(); i++)
	{
		if (passengersInElevator[i]->destination < floor)
		{
			return true;
		}
	}
	return false;
}
