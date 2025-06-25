#pragma once
#include <math.h>
#include <vector>
#include <time.h>
#include <queue>

class ElevatorLogic 
{
public:
	ElevatorLogic();

	struct passenger
	{
		int startFloor;
		int destination;
		bool isInElevator = false;
	};
	int elevatorLoop(time_t timeSinceStop);
	void addPassenger(int startFloor, int destination);
	int passengerCount(int floor) const { return floorPassengers[floor].size(); }
	std::vector<std::vector<passenger>> floorPassengers; // passengers on each floor
	std::vector<passenger*> passengersInElevator; // passengers currently in the elevator
private:
	int currentFloor = 0;
	bool goingUp = false; // true if elevator is going up, false if going down
	bool isDestinationAbove(int floor);
	bool isDestinationBelow(int floor);
};