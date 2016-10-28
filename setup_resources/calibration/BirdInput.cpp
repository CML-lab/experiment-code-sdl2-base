#include <cmath>
#include <windows.h>
#include "Bird.h"
#include "config.h"
#include "BirdInput.h"

BirdInput::BirdInput(int i)
{
	index = i;
}

InputFrame BirdInput::GetFrame()
{
	InputFrame input;
	BIRDREADING* bird_data = &frame.reading[index]; // Isolate the data for this specific bird
	
	// Make all time data relative to the first frame receieved.
	if (birds_start == 0)
	{
		birds_start = frame.dwTime;
	}

	/* The position and orientation data is stored in bird_data. Positions are
	 * stored as nX, nY, and nZ, and angles are stored as nAzimuth, nElevation,
	 * and nRoll. To convert the positions to meters, multiply by 0.9144 / 32768 (36 in = 32768).
	 * To convert orientations to radians, multiply by atan(1) * 4 / 32768 (180 deg = 32767). 
	 * Note, use atan(1)*4 to get to pi radians (or, 180 deg).
	 * These values can then be translated to the appropriate coordinate bounds.
	 *
	 * Keep in mind that the coordinate system used by the returned Flock of
	 * Birds data is different than the one used to output to the screen. "Up"
	 * on the screen is the -z axis for the Flock of Birds and +y for OpenGL.
	 * "Right" on the screen is the +y axis for the Flock of Birds and +x for
	 * OpenGL.
	 */
	//input.x = bird_data->position.nY * 0.9144f / 32767.f;
	//input.y = bird_data->position.nZ * 0.9144f / 32767.f;
	//input.theta = -(bird_data->angles.nRoll / 32767.f + 1.0f) * atan(1.0f) * 4;

	//convert to meters, and remove offset
	input.x = (bird_data->position.nY * 0.9144f / 32768.f) + CALxOFFSET;
	input.y = (bird_data->position.nZ * 0.9144f / 32768.f) + CALyOFFSET;

	//rotate axes to align display and flock of birds
	GLfloat tmpx, tmpy;
	tmpx = input.x;
	tmpy = input.y;
	input.x = tmpx*cos(CALxyROTANG) - tmpy*sin(CALxyROTANG);
	input.y = tmpx*sin(CALxyROTANG) + tmpy*cos(CALxyROTANG);

	//input.z = bird_data->position.nX * 0.9144f / 32768.f;

	
	//rotations: note, for now, these are uncalibrated! See AW for calibration data.

	input.theta = -(bird_data->angles.nRoll / 32768.f) * atan(1.0f) * 4; //display_yaw = bird_roll (rot about x axis)
	//input.pitch = -(bird_data->angles.nAzimuth / 32768.f) * atan(1.0f) * 4; //display_pitch = bird_azimuth/yaw (rot about z axis)
	//input.roll = -(bird_data->angles.nElevation / 32768.f) * atan(1.0f) * 4; //display_roll = bird_elevation/pitch (rot about y axis)
	input.time = frame.dwTime - birds_start;

	return input;
}

bool BirdInput::ProcessData()
{
	if (birdFrameReady(GROUP_ID))
	{
		birdGetFrame(GROUP_ID, &frame);
		//birdGetMostRecentFrame(GROUP_ID, &frame);
		return true;
	}
	else
	{
		return false;
	}
}