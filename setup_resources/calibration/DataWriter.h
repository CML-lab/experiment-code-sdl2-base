#ifndef DATAWRITER_H
#define DATAWRITER_H
#pragma once

#include <fstream>
#include "InputFrame.h"
//#include ".\Flock_of_Birds\Bird.h"
#include "TrackBird.h"

// Records data
class DataWriter
{
private:
	std::ofstream file;
public:
	/* filename defines a custom name to save the file as. If filename is NULL,
	 * a name will be generated automatically.
	 */
	DataWriter(TrackSYSCONFIG *sysconfig, const char* filename = NULL);
	~DataWriter();
	// Record a new line of data
	// deviceNo is a number identifying the device sending the data
	// frame is the frame of input data
	void Record(int deviceNo, InputFrame frame);
};

#endif
