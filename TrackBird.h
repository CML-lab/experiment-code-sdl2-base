/*
   This object controls interactions with the Ascension tracker systems, both
   the TrakSTAR and the Flock of Birds. It is designed to be used on either system,
   and requires only specification of a flag (e.g., set in config.h) indicating
   which tracker is currently connected to the machine. Unfortunately, it is not
   able to automatically detect the Ascension system at this time.

   Additional parameters may be set as flags in config.h including specification
   as to which filters are to be turned on/off. Unless otherwise specified, the
   tracker is initiated in polling mode (rather than streaming mode) as there is
   currently no code to handle buffering of streamed data.
   
   Errors are written out to the standard error output stream (std::cerr), which 
   is typically directed to a text file in the Debug folder.

   This object utilizes a standardized data frame, TrackDATAFRAME, which is defined
   in the InputFrame object.

   This object requires three function calls: initialization of the tracking system,
   polling for data, and shutting down the tracking system. Shutting down the system
   properly is essential to ensure that the tracker does not end in a strange state
   and that when possible, the transmitter is shut down to avoid overheating.

*/

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