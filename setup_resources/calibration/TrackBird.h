#ifndef TRACKBIRD_H
#define TRACKBIRD_H
#pragma once

#include "config.h"
#include <windows.h>

#include "Flock_of_Birds\Bird.h"
#include "trakSTAR\ATC3DG.h"


//system constants
#define FOB_READ_TIMEOUT  2000
#define FOB_WRITE_TIMEOUT  2000

//bird count: 4 for Kinereach, 1 or 2 for trakSTAR
//#define BIRDCOUNT  4 

static double birds_start = 0;

struct TrackDATAFRAME
{
	
	double x[BIRDCOUNT+1];
	double y[BIRDCOUNT+1];
	double z[BIRDCOUNT+1];
	//double azimuth[BIRDCOUNT+1];
	//double elevation[BIRDCOUNT+1];
	//double roll[BIRDCOUNT+1];
	double anglematrix[BIRDCOUNT+1][3][3];

	double time[BIRDCOUNT+1];  //this is time according to the tracking system, arbitrary according to the system clock
	double etime[BIRDCOUNT+1];  //this is elapsed time from the start of data recording
	double quality[BIRDCOUNT+1];

};

struct TrackSYSCONFIG
{
	//general parameters
	double trackType;
	double measureRate;
	USHORT SensorIDs[BIRDCOUNT+1];  //for FOB these are the COM ports; for trakSTAR these are sensor numbers
	int BirdCount;
	BOOL filter_wide;
	BOOL filter_narrow;
	double filter_dc;

	//FOB parameters
	int GroupID;

	//trakSTAR parameters
	double reportRate;
	bool metric;
	int datatype;
	int NBirdsActive;
} ;




class TrackBird
{
private:

public:
	~TrackBird() { }
	
	static int InitializeBird(TrackSYSCONFIG *);  //initialize the system; return 0 for failed attempt, otherwise 1 for FOB or 2 for trakSTAR
	static int GetUpdatedSample(TrackSYSCONFIG *,TrackDATAFRAME *);  //poll for a new sample for a given sensor.  return number of new samples available, or 0 if no update was made
	static bool ShutDownBird(TrackSYSCONFIG *);    //shut down the system; return false for failed attempt, otherwise true

};




// Retrieves inputs from a bird sensor


#endif