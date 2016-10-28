#ifndef INPUTFRAME_H
#define INPUTFRAME_H
#pragma once

#include "SDL_opengl.h"


// Data type used to return input data
struct TrackDATAFRAME
{
	int ValidInput;	

	double x;		
	double y;		
	double z;		

	double anglematrix[3][3]; 
	double theta;

	double time;	//this is time according to the tracking system, arbitrary according to the system clock
	double etime;	//this is elapsed time from the start of data recording
	double quality;	

	float vel;

};


#endif
