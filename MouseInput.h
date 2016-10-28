#ifndef MOUSEINPUT_H
#define MOUSEINPUT_H
#pragma once

#include "SDL.h"
#include "InputFrame.h"

static GLfloat x;
static GLfloat y;

// Handles mouse actions
class MouseInput
{
public:
	MouseInput() { }
	~MouseInput() { }
	// Gets the most recent frame of data from the mouse
	static int GetFrame(TrackDATAFRAME DataMouseFrame[]);
	// Updates MouseInput with new position information.
	// event is an SDL_Event containing the updated information.
	static void ProcessEvent(SDL_Event event);
};

#endif
