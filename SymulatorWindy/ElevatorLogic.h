#pragma once
#include <math.h>
#include <vector>
#include "GUI.h"
#include <time.h>
#include <queue>
#include <algorithm>

constexpr int MAX_CAPACITY = 8; // Maximum number of passengers in the elevator
constexpr int SPACING = 24; // Spacing between passengers in the elevator
constexpr int OFFSET_BASE = 24; // Base offset for repositioning passengers on the floor
constexpr int LEFT_X = 253; // X position for left side of the floor
constexpr int RIGHT_X = 500; // X position for right side of the floor
constexpr int ANIMATION_SPEED_PX_PER_SEC = 100; // Speed of passenger animations in pixels per second
constexpr int ANIMATION_DELAY_MS = 1; // Delay after moving the elevator sprite
constexpr int IDLE_THRESHOLD = 5; // Time in seconds after which the elevator returns to ground floor if idle

struct passenger
{
	int startFloor;
	int destination;
	bool isInElevator = false;
	size_t passengerId;
};

struct elevator
{
	size_t elevatorId;
};

class ElevatorLogic 
{
public:
	ElevatorLogic(GdiplusWindow* window_);

	bool elevatorLoop(time_t timeSinceStop, bool wasEmpty);
	void addPassenger(int startFloor, int destination, size_t spriteId);
	int passengerCount(int floor) const { return floorPassengers[floor].size(); }

private:
	GdiplusWindow* window; // Pointer to the GUI window for drawing
	elevator* elevatorData;
	COORD textPosition = { 300, 25 }; // Position for the text displaying passenger weight
	size_t textId;
	int currentFloor = 0;
	bool goingUp = false; // true if elevator is going up, false if going down
	bool isDestinationAbove(int floor);
	bool isDestinationBelow(int floor);
	std::vector<std::vector<passenger*>> floorPassengers; // passengers on each floor
	std::vector<passenger*> passengersInElevator; // passengers currently in the elevator

	void loadPassengersAtCurrentFloor();
	void repositionFloorQueue(const std::vector<passenger*>& queue);
	void unloadPassengersAtCurrentFloor();
	bool updateDirection(time_t timeSinceStop, bool wasEmpty);
	void handleIdleBehavior(time_t timeSinceStop);
	void animatePassengersInElevator();
	void moveElevatorSprite();

};