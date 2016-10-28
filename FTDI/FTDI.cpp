#include <cmath>
#include <windows.h>
#include <iostream>
#include "FTDI.h"


int Ftdi::InitFtdi(int i, FT_HANDLE *ftHandle, int devMode, UCHAR mask)
{
	DWORD numDevs;
	FT_STATUS status;
	//DWORD Flags;
	//DWORD ID;
	//DWORD Type;
	//DWORD LocId;
	//char SerialNumber[16];
	//char Description[64]; 
	UCHAR XORMask = 0x0f;

	status = FT_CreateDeviceInfoList(&numDevs);
	if (status != FT_OK)
	{
		std::cerr << "FTDI device list not obtained." << std::endl;
		return(1);
	}

	//Got a valid number of devices back.  
	if (i > numDevs)
	{
		std::cerr << "Requested FTDI device " << i << " is not available or has not been found." << std::endl;
		return (2);
	}
	else
	{
		// get the device information list
		//status = FT_GetDeviceInfoDetail(i, &Flags, &Type, &ID, &LocId, SerialNumber,Description,&ftHandle); 
		//if (status == FT_OK)
		//	std::cerr << "FTDI device " << i << " found: " << ID << std::endl;

		//open the device
		status = FT_Open(i,ftHandle);
		if (status == FT_OK)
			std::cerr << "FTDI device " << i << " opened." << std::endl;
		else
		{
			std::cerr << "FTDI device " << i << " failed to open." << std::endl;
			return (3);
		}

		if (devMode == 1)
		{

			//UCHAR Mask = 0x00;
			UCHAR Mode = 0x01;

			//printf("mask: %x %x %x %x, Mask: %x\n",0xff,0xf0,0x0f,0x00,mask);

			//the bits in the upper nibble of the mask should be set to 1 to be output lines and 0 to be input lines.

			mask = mask ^ XORMask;  //we invert the bits in the lower half of the mask because the set logic is inverted (so if we want a bit to go high, we have to send a 0 to the FTDI)
	
			status = FT_SetBitMode(*ftHandle, mask, Mode); 
			if (status == FT_OK) 
			{ 
				//printf("Set in BitBang Mode.\n");
				std::cerr << "FTDI device " << i << " set in BitBang mode." << std::endl;
				std::cerr << "FTDI device Mask: ";
				std::cerr << std::hex << (mask ^ XORMask) << std::dec << std::endl;
			} 
			else 
			{ 
				// FT_SetBitMode FAILED!
				//printf("Failed to set Mode.\n");
				std::cerr << "FTDI device " << i << " failed to set in BitBang mode." << std::endl;
				return (4);
			}

		}
	}
	
	return(0);


}

bool Ftdi::CloseFtdi(FT_HANDLE ftHandle, int devMode)
{
	FT_STATUS status;
	
	if (devMode == 1)
	{
		//reset the bitmode defaults
		UCHAR Mask = 0xf5;
		UCHAR Mode = 0x00;  //reset mode
		status = FT_SetBitMode(ftHandle,Mask,Mode);

	}
	

	status = FT_Close(ftHandle);

	if (status == FT_OK)
		return (true);
	else
		return(false);

}


int Ftdi::GetFtdiData(FT_HANDLE ftHandle)
{

	FT_STATUS status;

	DWORD dwModemStatus = 0;
	DWORD dwLineStatus = 0;
	DWORD CTSstatus = 0;

	status = FT_GetModemStatus(ftHandle, &dwModemStatus);
	if (status == FT_OK)
	{
		//data is received

		// Line status is the second byte of the dwModemStatus value
		dwLineStatus = ((dwModemStatus >> 8) & 0x000000FF);
		// Now mask off the modem status byte
		dwModemStatus = (dwModemStatus & 0x000000FF);

		//our data input is being put out on the CTS line, which is bit-mapped to 0x10 in the least significant byte of the lpdwModemStatus
		//we extract this here:
		CTSstatus = (dwModemStatus & 0x00000010);
		//return(CTSstatus);  

		//the CTS line floats high.
		//when you ground the CTS line, the bit reads true so the logic is reversed.  So we reverse it back here so 0 means zero voltage.
		if (CTSstatus == 0)  
			return(1);
		else //if (CTSstatus > 0)
			return(0);

	}
	else
		std::cerr << "FTDI: Read CTS failed." << std::endl;

	return(-1);
}


int Ftdi::GetFtdiBitBang(FT_HANDLE ftHandle,int bit)
{

	FT_STATUS status;
	UCHAR dataBit;

	status = FT_GetBitMode(ftHandle, &dataBit);  //get the value of the data bus
	if (status == FT_OK)
	{
		//data is received
		
		//return(dataBit);

		
		//our data input at the bit specified in the function call. 
		if (bit == -1 || bit == 4)  //this is the default, or the CTS line, which is located on bit 4
		{
			
			if ((dataBit & 0x08) == 0x00)
				return(0);
			else
				return(1);
		}
		else if (bit == 1)
		{
			if ((dataBit & 0x01) == 0x00) //(dataBit == 0xfe)
				return(0);
			else
				return(1);
		}
		else if (bit == 2)
		{
			if ((dataBit & 0x02) == 0x00) //(dataBit == 0xfd)
				return(0);
			else
				return(1);
		}
		else if (bit == 3)
		{
			if ((dataBit & 0x04) == 0x00) //(dataBit == 0xfb)
				return(0);
			else
				return(1);
		}
		else
		{
			std::cerr << "FTDI: Unsure how to read data bit " << bit << "." << std::endl;
			return(-2);
		}

	}

	else
		std::cerr << "FTDI: Read data bit " << bit << " failed." << std::endl;

	return(-3);
}

int Ftdi::SetFtdiBitBang(FT_HANDLE ftHandle,UCHAR mask, int bit, int value)
{
	
	FT_STATUS status;
	UCHAR Mode = 0x01;
	UCHAR Mask;
	UCHAR dataByte;
	int rtn = 0;
	UCHAR XORMask = 0x0f;

	status = FT_GetBitMode(ftHandle, &Mask);  //we do this to preserve the lower nibble, which are the data lines
	Mask = Mask & 0x0f;  //we isolate the lower nibble (data lines)
	Mask = Mask ^ XORMask;  //we have to invert these lower bits because the SET logic is inverted; this preserves the data lines
	mask = mask & 0xf0;  //we isolate the upper nibble of the input mask, which tells us which lines are inputs (0) and which are outputs (1).
	//mask = mask & (mask >> 4); //any mask lines that are inputs shall be reset to be high in the lower nibble, preserving their state as active inputs. if they are set to zero, this disables the input lines.
	Mask = Mask | mask;  //now we combine the upper nibble of the mask and the lower nibble of the data lines
	Mask = Mask & (0xf0 | Mask >> 4); //any mask lines that are inputs shall be reset to be active (low by the inverted logic) in the lower nibble, preserving their state as active inputs. if they are set to one (inverted logic), this disables the input lines.

	printf("\nMask1: %x\n",Mask);

	if (bit == 1)
	{
		if (Mask & 0x10)  //check that the line is a write-out line
		{
			if (value == 1)
				Mask = Mask & 0xfe;  //set to zero to flip the bit high -- set logic is inverted!
			else //value == 0
				Mask = Mask | 0x01;	 //set to one to flip the bit low -- set logic is inverted!
			rtn = 0;
		}
		else
			return(-1); //status: requested line write-out is not an output line

	}
	else if (bit == 2)
	{
		if (Mask & 0x20)  //check that the line is a write-out line
		{
			if (value == 1)
			{
				Mask = Mask & 0xfd;
			}
			else //value == 0
			{
				Mask = Mask | 0x02;
			}
			rtn = 0;
		}
		else
			return(-1); //status: requested line write-out is not an output line

	}
	else if (bit == 3)
	{
		if (Mask & 0x40)  //check that the line is a write-out line
		{
			if (value == 1)
				Mask = Mask & 0xfb;
			else //value == 0
				Mask = Mask | 0x04;
			rtn = 0;
		}
		else
			return(-1); //status: requested line write-out is not an output line

	}
	else if (bit == 4)
	{
		if (Mask & 0x80)  //check that the line is a write-out line
		{
			if (value == 1)
				Mask = Mask & 0xf7;
			else //value == 0
				Mask = Mask | 0x08;
			rtn = 0;
		}
		else
			return(-1); //status: requested line write-out is not an output line
	}

	printf("SetMask: %x\n",Mask ^ XORMask);

	status = FT_SetBitMode(ftHandle, Mask, Mode); 
	if (status == FT_OK) 
	{ 
		status = FT_GetBitMode(ftHandle, &Mask); 
		//printf("DataByte: %x\n",Mask);
		std::cerr << "FTDI Bits set: " ;
		std::cerr << std::hex << Mask << std::dec << std::endl;

		//std::cerr << "FTDI device bits set." << std::endl;
		return (0);
	} 
	else 
	{ 
		// FT_SetBitMode FAILED!
		//printf("Failed to set bits.\n");
		std::cerr << "FTDI device failed to set bits." << std::endl;
		return (-4);
	}
	
	return (rtn);

}