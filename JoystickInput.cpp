#include "config.h"
#include "JoystickInput.h"
#include <iostream>

int JoystickInput::GetFrame(TrackDATAFRAME dataframe[], int input)
{
	dataframe[input].x = jx;
	dataframe[input].y = jy;

	dataframe[input].z = jradius;
	dataframe[input].theta = jangle;

	dataframe[input].time = SDL_GetTicks();

	return(1);
}

void JoystickInput::ProcessEvent(SDL_Event event, SDL_Joystick *joystick)
{
	float x_norm, y_norm, x_trans, y_trans;
	float deadzone = 3200.0f;  //dead zone around the center of the joystick that must be exceeded to register a real "extent".  


	//the joystick is 16 bit signed, so it ranges from from -32767 to 32768.

	if (event.type == SDL_JOYAXISMOTION)
	{
		//update the current joystick status
		SDL_JoystickUpdate();

        /* 
			Read in the axis motion data:
			
			In this function, no manipulation of the input will be performed, we just read in the raw values as is.
			Scaling, dead zones, etc. will be handled in the GetFrame() call.

			Left-right movement: left = -32768, right = 32767
			Up-Down movement: up = -32768, down = 32767 

		*/

		jx = SDL_JoystickGetAxis(joystick, 0);  //axis0 is left/right
		jy = SDL_JoystickGetAxis(joystick, 1);  //axis1 is up/down

		if (jx > deadzone)  //positive and larger than deadzone
			x_norm = (jx - deadzone)/(32767.0f - deadzone);
		else if (jx < -(deadzone + 1))  //negative and smaller than deadzone
			x_norm = (jx + (deadzone + 1))/(32768.0f - (deadzone + 1));
		else  //within the deadzone; this is treated as a zero value
			x_norm = 0.0f;
	
		if (jy > deadzone)  //positive and larger than deadzone
			y_norm = (jy - deadzone)/(32767.0f - deadzone);  
		else if (jy < -(deadzone + 1))  //negative and smaller than deadzone
			y_norm = (jy + (deadzone+1))/(32768.0f - (deadzone + 1));
		else  //within the deadzone; this is treated as a zero value
			y_norm = 0.0f;
	
		y_norm = -y_norm;  //reverse the sign of y to get a standard angular position

		//we transform the rectangular joystick coordinates into circular polar coordinates by first performing a spatial transformation to warp the hand space.
		//this is done according to the proof here: http://mathproofs.blogspot.com/2005/07/mapping-square-to-circle.html
		x_trans = x_norm*sqrt(1 - (y_norm*y_norm)/2);
		y_trans = y_norm*sqrt(1 - (x_norm*x_norm)/2);
		//note this is not quite ideal because the rectangular "dead zone" extracted from the middle of the square is not transformed into a circle, 
		//  but more of a squarish-thing-with-curved-edges.  but for now, it's good enough.

		//save the normalized positions of the two axes
		jx = x_norm;
		jy = y_norm;

		//calculate the normalized radius position
		jradius = sqrt(x_trans*x_trans + y_trans*y_trans); //z reports the normalized, transformed radial position of the joystick (minus the square deadzone) in polar coordinates

		//calculate the angular position
		jangle = atan2f(y_trans,x_trans); //theta reports the angular position of the joystick (outside the deadzone) in polar coordinates

	}// end event is axismotion
	
}


void JoystickInput::CloseJoystick(SDL_Joystick *joystick)
{
	SDL_JoystickClose(joystick);
}