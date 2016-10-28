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
	stringstream ss1,ss2;
	ss1 << setw(4) << setfill('0') << ltm->tm_year + 1900;
	ss1 << setw(2) << setfill('0') << ltm->tm_mon + 1;
	ss1 << setw(2) << setfill('0') << ltm->tm_mday;
	//ss << "_";
	ss2 << setw(2) << setfill('0') << ltm->tm_hour;
	ss2 << setw(2) << setfill('0') << ltm->tm_min;
	ss2 << setw(2) << setfill('0') << ltm->tm_sec;


	// If no filename was supplied, use the date and time
	if (filename == NULL)
	{
		stringstream ss3;
		ss3 << ss1.str() << "_" << ss2.str() << ".txt";
		file.open(ss3.str(), ios::out);
	}
	else
	{
		//append timestamp to prevent overwrites
		stringstream ss3;
		ss3 << ss1.str() << ss2.str() << "_" << filename;
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
		file << "Number_of_Birds " << sysconfig->BirdCount << endl;
		file << "Sampling_Rate " << sysconfig->measureRate << endl;
		file << "Report_Rate " << sysconfig->reportRate << endl;

		file << "--" << endl;
		
		file << "Device_Num "
			<< "FakeTime "
			<< "HandX "
			<< "HandY "
			<< "HandZ "
			<< "HandAngMat1 "
			<< "HandAngMat2 "
			<< "HandAngMat3 "
			<< "HandAngMat4 "
			<< "HandAngMat5 "
			<< "HandAngMat6 "
			<< "HandAngMat7 "
			<< "HandAngMat8 "
			<< "HandAngMat9 "
			<< endl;

		file << "-----" << endl;  //flag designator for finding start of the data stream.  everything above is header

	}
}

DataWriter::~DataWriter()
{
	file.close();
}

void DataWriter::Record(int deviceNo, InputFrame frame)
{
	// Write data
	if (file.is_open())
	{
		file << deviceNo << " "
			<< frame.time << " "
			<< frame.x << " "
			<< frame.y << " "
			<< frame.z << " "
			<< frame.angmatrix[0] << " "
			<< frame.angmatrix[1] << " "
			<< frame.angmatrix[2] << " "
			<< frame.angmatrix[3] << " "
			<< frame.angmatrix[4] << " "
			<< frame.angmatrix[5] << " "
			<< frame.angmatrix[6] << " "
			<< frame.angmatrix[7] << " "
			<< frame.angmatrix[8] << " "
			<< endl;
	}
}