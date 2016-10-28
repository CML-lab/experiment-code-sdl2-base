//Modified to add target inputs, 9/10/2012 AW

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <istream>
#include "TrialTable.h"


int TrialTable::LoadTrFile(char* filename)
{

	char tmpline[40] = ""; 
	int ntrials = 0;

	std::ifstream trfile(filename);

	//trfile.open(fname);
	if (!trfile)
	{
		std::cerr << "Cannot open input file." << std::endl;
		return -1;
	}

	trfile.getline(tmpline,sizeof(tmpline),'\n');

	while(!trfile.eof())
	{
			sscanf(tmpline, "%d %f %f %d %d",&TrialTable::trtbl[ntrials].NewTrial,&TrialTable::trtbl[ntrials].xpos, &TrialTable::trtbl[ntrials].ypos, &TrialTable::trtbl[ntrials].duration, &TrialTable::trtbl[ntrials].trace);
			ntrials++;
			trfile.getline(tmpline,sizeof(tmpline),'\n');
	}

	trfile.close();
	if(ntrials == 0)
	{
		std::cerr << "Empty input file." << std::endl;
		return -1;
	}
	return ntrials;
}


