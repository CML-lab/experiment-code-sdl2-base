/*
   This object controls interactions with the Codamotion optical tracking system.
   It is designed to be used in place of the TrackBird object.

   Errors are written out to the 
   standard error output stream (std::cerr), which is typically directed to a
   text file in the Debug folder.

   This object utilizes a standardized data frame, TrackDATAFRAME, which is defined
   in the InputFrame object.

*/

#ifndef TRACKCODAMOTION_H
#define TRACKCODAMOTION_H
#pragma once

#define NOWRITEDATA  0  //don't decode data and don't write data to (text) file.
#define SERVER 0

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <istream>
#include <fstream>
#include <sstream>
#include <memory.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <map>
#include "InputFrame.h"
#include "config.h"
#include "Timer.h"

// RTNet C++ includes
#include "codaRTNetProtocolCPP/RTNetClient.h"
#include "codaRTNetProtocolCPP/DeviceOptionsAlignment.h"
#include "codaRTNetProtocolCPP/DeviceOptionsCodaMode.h"
#include "codaRTNetProtocolCPP/DeviceOptionsCodaPacketMode.h"
#include "codaRTNetProtocolCPP/DeviceInfoAlignment.h"
#include "codaRTNetProtocolCPP/DeviceInfoUnitCoordSystem.h"
#include "codaRTNetProtocolCPP/PacketDecode3DResult.h"
#include "codaRTNetProtocolCPP/PacketDecode3DResultExt.h"
#include "codaRTNetProtocolCPP/PacketDecodeADC16.h"
#include "codaRTNetProtocol/codartprotocol_gs16aio.h"
#include "codaRTNetProtocol/codartprotocol_ni6221.h"
#include "codaRTNetProtocol/codartprotocol_cx1.h"


struct CODASYSCONFIG
{
	codaRTNet::RTNetClient cl;  //client connection object
	codaRTNet::AutoDiscover discover;
	codaRTNet::DataStream stream;
	FILE* monitorfile;
	
	DWORD configchoice;
	BOOL bExtSync;

	int ncameras;

	//device ID flags
	bool bGS16AIOenabled;
	bool bNI6221enabled;

    int MonitorPeriod; //ms
	static const int UDPbufferSize = 100000;

	DWORD cx1mode;
    DWORD cx1decim;
	int MaxMarkers;
	WORD MaxMarkerInUse;

	int MonitorRate;  //effective sampling rate during the task
	float AcqRate;		//true data samplng rate in the final saved dataset
    DWORD MaxSamples;
    float AcqTimeMax;
	DWORD PacketSize;
	DWORD PacketTransmitSize;

    // acquisition counters & timers:
	int MonitorSample;
    DWORD SampleNum;
    DWORD PrevSampleNum;
    float SampleTime;
    DWORD SyncStopSampleNum[1000];

	float cx1TickTime;		//sample tick period for cx1 data (should be same as AcqRate)

	clock_t timeStart;
    clock_t timeStop;

	Timer* GetSampleTimer;

};




class TrackCoda
{
private:
	
public:
	~TrackCoda() { };
	
	static int InitializeCoda(CODASYSCONFIG *CodaSysConfig);  // initialize the system; return 0 for failed attempt, otherwise 1 for FOB or 2 for trakSTAR
	static int GetUpdatedSample(CODASYSCONFIG *CodaSysConfig, TrackDATAFRAME DataBirdFrame[]);  // poll for a new sample for a given sensor.  return number of new samples available, or 0 if no update was made
	static int ShutDownCoda(CODASYSCONFIG *CodaSysConfig,tm* ltm);    // shut down the system; return false for failed attempt, otherwise true

	static void print_alignment_status(const DWORD* marker_id_array,  const codaRTNet::DeviceInfoAlignment& info);
	static void print_devicestatusarray_errors(const codaRTNet::DeviceStatusArray& array);
	static DWORD getfirstdeverror(const codaRTNet::DeviceStatusArray& arr);

};



#endif


