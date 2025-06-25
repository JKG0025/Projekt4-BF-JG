#include "ElevatorLogic.h";

ElevatorLogic::ElevatorLogic()
{
	floorPassengers.resize(5);
}

int ElevatorLogic::elevatorLoop(time_t timeSinceStop)
{
	for (int i = 0; i < passengersInElevator.size(); i++)
	{
		if (passengersInElevator[i]->destination == currentFloor)
		{
			passengersInElevator[i]->isInElevator = false;
			passengersInElevator.erase(passengersInElevator.begin() + i);
			i--;
		}
	}
	for (int i = 0; i < floorPassengers[currentFloor].size(); i++)
	{
		if (passengersInElevator.size() < 7 && goingUp ? floorPassengers[currentFloor][i].destination > currentFloor : floorPassengers[currentFloor][i].destination < currentFloor)
		{
			passengersInElevator.push_back(&floorPassengers[currentFloor][i]);
			floorPassengers[currentFloor][i].isInElevator = true;
			floorPassengers[currentFloor].erase(floorPassengers[currentFloor].begin() + i);
			i--;
		}
	}
	if (goingUp)
	{
		if (isDestinationAbove(currentFloor))
		{
			if (currentFloor != 4)
			currentFloor++;
		}
		else if (isDestinationBelow(currentFloor))
		{
			goingUp = false;
			if (currentFloor != 0)
			currentFloor--;
		}
		else if (passengersInElevator.empty() && timeSinceStop > 5)
		{
			goingUp = false; // stop the elevator if there are no passengers and it has been stopped for more than 5 seconds
			if (currentFloor != 0)
			currentFloor--;
		}
	}
	else
	{
		if (isDestinationBelow(currentFloor))
		{
			if (currentFloor != 0)
			currentFloor--;
		}
		else if (isDestinationAbove(currentFloor))
		{
			goingUp = true;
			if (currentFloor != 4)
				currentFloor++;
		}
		else if (passengersInElevator.empty() && timeSinceStop > 5)
		{
			goingUp = false;
			if (currentFloor != 0)
			currentFloor--;
		}
	}
	
	return currentFloor;
}

void ElevatorLogic::addPassenger(int startFloor, int destination)
{
	if (startFloor < 0 || startFloor > 4 || destination < 0 || destination > 4 || startFloor == destination)
	{
		return; // invalid floor or same floor
	}

	passenger newPassenger;
	newPassenger.startFloor = startFloor;
	newPassenger.destination = destination;
	newPassenger.isInElevator = false;
	floorPassengers[startFloor].push_back(newPassenger);
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
