#ifndef MOUSEINPUT_H
#define MOUSEINPUT_H
#pragma once

#include "SDL.h"
#include "InputDevice.h"

static GLfloat x;
static GLfloat y;

// Handles mouse actions
class MouseInput : public InputDevice
{
public:
	MouseInput() { }
	~MouseInput() { }
	// Gets the most recent frame of data from the mouse
	InputFrame GetFrame();
	// Updates MouseInput with new position information.
	// event is an SDL_Event containing the updated information.
	static void ProcessEvent(SDL_Event event);
};

#endif
