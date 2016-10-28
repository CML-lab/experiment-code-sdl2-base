//Modified to add target inputs, 9/10/2012 AW

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "DataWriter.h"


using namespace std;

DataWriter::DataWriter(TrackSYSCONFIG *sysconfig, const char* filename)
{

	//record current date/time
	time_t current_time = time(0);
	tm* ltm = localtime(&current_time);
	stringstream ss1, ss2, ss3;
	
	ss1 << setw(4) << setfill('0') << ltm->tm_year + 1900;
	ss1 << setw(2) << setfill('0') << ltm->tm_mon + 1;
	ss1 << setw(2) << setfill('0') << ltm->tm_mday;
	
	ss2 << setw(2) << setfill('0') << ltm->tm_hour;
	ss2 << setw(2) << setfill('0') << ltm->tm_min;
	ss2 << setw(2) << setfill('0') << ltm->tm_sec;


	// If no filename was supplied, use the date and time
	if (filename == NULL)
	{
		ss3 << ss1.str() << "_" << ss2.str() << ".dat";
		file.open(ss3.str(), ios::out);
	}
	else
	{
		//append timestamp to prevent overwrites
		ss3 << filename << "_" << ss1.str() << ss2.str() << ".dat";
		file.open(ss3.str(), ios::out);
	}

	// Write headers to file
	if (file.is_open())
	{

		//write current date
		file << "Date " << ss1.str() << endl;
		file << "Time " << ss2.str() << endl;
		
		//write bird config parameters of interest
		file << "Tracker: " << (sysconfig->trackType ? "TrakStar" : "FOB") << endl;
		file << "Number_of_Birds " << sysconfig->birdCount << endl;
		file << "Sampling_Rate " << sysconfig->measureRate << endl;
		file << "Report_Rate " << sysconfig->reportRate << endl;

		file << "--" << endl;
		
		file << "Device_Num "
			 << "TrackerTime "
			 << "Trial "
			 << "HandX "
			 << "HandY "
			 << "Velocity "
			 << "StartX "
			 << "StartY "
		     << "TargetX "
			 << "TargetY "
		     << "TargetX2 "
			 << "TargetY2 "
			 << "TrType "
			 << "VelMin "
			 << "VelMax "
			 << "TrialScore "
			 << endl;

		file << "-----" << endl;  //flag designator for finding start of the data stream.  everything above is header

	}
}

DataWriter::~DataWriter()
{
	file.close();
}
 
void DataWriter::Record(int deviceNo, TrackDATAFRAME frame, TargetFrame Target)
{
	// Write data
	if (file.is_open())
	{
		file << deviceNo << " "
			<< std::fixed << showpoint << std::setprecision(5) 
			<< frame.time << " "
			<< std::resetiosflags( std::ios::fixed | std::ios::showpoint )
			<< Target.trial << " "
			<< frame.x << " "
			<< frame.y << " "
			<< frame.vel << " "
			<< Target.startx << " "
			<< Target.starty << " "
			<< Target.tgtx << " "
			<< Target.tgty << " "
			<< Target.tgtx2 << " "
			<< Target.tgty2 << " "
			<< Target.TrType << " "
			<< Target.velmin << " "
			<< Target.velmax << " "
			<< Target.score << " "
			<< endl;
	}
}

void DataWriter::Close()
{
	file.close();
}