#ifndef FTDI_H
#define FTDI_H
#pragma once

#include "ftd2xx.h"

// Handles mouse actions
class Ftdi
{
public:
	Ftdi() { }
	~Ftdi() { }
	//initialize sensor
	static int InitFtdi(int i, FT_HANDLE *ftHandle, int devMode = 1, UCHAR mask = 0x00);
	//get and process data from sensor
	static int GetFtdiData(FT_HANDLE ftHandle);
	static int GetFtdiBitBang(FT_HANDLE ftHandle,int bit = -1);
	static int SetFtdiBitBang(FT_HANDLE ftHandle, UCHAR mask, int bit, int value);  //set lines on the ftdi
	//shutdown sensor
	static bool CloseFtdi(FT_HANDLE ftHandle, int devMode = 1);

};

#endif
