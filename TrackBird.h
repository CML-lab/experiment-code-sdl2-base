#ifndef TRACKBIRD_H
#define TRACKBIRD_H
#pragma once

#include "config.h"
#include <windows.h>

#include "Flock_of_Birds\Bird.h"
#include "trakSTAR\ATC3DG.h"
#include "InputFrame.h"

//system constants
#define FOB_READ_TIMEOUT  2000
#define FOB_WRITE_TIMEOUT  2000

static double birds_start = 0;

struct TrackSYSCONFIG
{
	//general parameters
	double trackType;
	double measureRate;
	USHORT SensorIDs[BIRDCOUNT + 1];  // for FOB these are the COM ports; for trakSTAR these are sensor numbers
	int birdCount;
	BOOL filter_wide;
	BOOL filter_narrow;
	double filter_dc;
	ADAPTIVE_PARAMETERS alpha_parameters;

	//FOB parameters
	int groupID;

	//trakSTAR parameters
	WORD reportRate;
	BOOL metric;
	int datatype;
	int nBirdsActive;
} ;




class TrackBird
{
private:

public:
	~TrackBird() { }
	
	static int InitializeBird(TrackSYSCONFIG *);  // initialize the system; return 0 for failed attempt, otherwise 1 for FOB or 2 for trakSTAR
	static int GetUpdatedSample(TrackSYSCONFIG *,TrackDATAFRAME DataBirdFrame[]);  // poll for a new sample for a given sensor.  return number of new samples available, or 0 if no update was made
	static bool ShutDownBird(TrackSYSCONFIG *);    // shut down the system; return false for failed attempt, otherwise true

};

#endif