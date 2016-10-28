#ifndef TRIALTABLE_H
#define TRIALTABLE_H
#pragma once

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <istream>


// Trial table.
typedef struct {
	int NewTrial;			// Flag 1-for new trial.
	float xpos,ypos;		// x/y pos of target.
	int duration;			// duration of target.
	int trace;
} TRTBL;

#define TRTBL_SIZE 1000


// Loads data
class TrialTable
{

public:

	static TRTBL trtbl[TRTBL_SIZE];

	// filename defines a name to read the file as. 
	static int LoadTrFile(char* filename);
};

#endif
