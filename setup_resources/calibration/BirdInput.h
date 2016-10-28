#ifndef BIRDINPUT_H
#define BIRDINPUT_H
#pragma once

#include "InputDevice.h"

static BIRDFRAME frame;
static DWORD birds_start = 0;

// Retrieves inputs from a bird sensor
class BirdInput : public InputDevice
{
private:
	int index;
public:
	// i defines the index of the bird represented
	BirdInput(int i);
	~BirdInput() { }
	// Gets the most recent frame of data from the bird
	InputFrame GetFrame();
	/* Checks whether the Flock of Birds has a new frame of data. If it does,
	 * update BirdInput and return true.
	 */
	static bool ProcessData();
};

#endif
