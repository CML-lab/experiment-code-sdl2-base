#ifndef READTRIALFILE_H
#define READTRIALFILE_H
#pragma once

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <istream>

// Records data
class ReadTrFile
{
private:
	int NewTrial;			// Flag 1-for new trial.
	float xpos,ypos;		// x/y pos of target.
	int duration;			// duration of target.
	int trace;
	//std::ifstream file;
public:
	// filename defines a name to read the file as. 
	int LoadTrFile(char* filename);
};

#endif
