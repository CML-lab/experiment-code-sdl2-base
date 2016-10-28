#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H
#pragma once

#include "InputFrame.h"

// Abstract class used to represent an input device
class InputDevice
{
public:
	// Gets the most recent frame of data from the device
	virtual InputFrame GetFrame() = 0;
};

#endif
