#ifndef JOYSTICKINPUT_H
#define JOYSTICKINPUT_H
#pragma once

#include "SDL.h"
#include "InputFrame.h"

static GLfloat jx;
static GLfloat jy;
static GLfloat jradius;
static GLfloat jangle;


// Handles joystick actions
class JoystickInput
{
public:
	JoystickInput() { }
	~JoystickInput() { }
	// Gets the most recent frame of data from the joystick
	static int GetFrame(TrackDATAFRAME dataframe[], int input);
	// Updates JoystickInput with new axes information.
	// event is an SDL_Event containing the updated information.
	static void ProcessEvent(SDL_Event event, SDL_Joystick *joystick);
	//close the joystick when done
	static void CloseJoystick(SDL_Joystick *joystick);
};

#endif
