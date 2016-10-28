#include <stdio.h>
#include <windows.h>

#include <windows.h>
//#include "PhotoSensor.h"
#include "FTDI.h"

#pragma pack(1)

int main()
{
printf("Hello world.\n");

FT_HANDLE ftHandle;
int status = -5;
int devNum = 0;
int data1 = 0;
int data2 = 0;
int data3 = 0;
int data4 = 0;

FT_STATUS ftStatus; 
DWORD numDevs;

UCHAR Mask = 0x0f;  //the bits in the upper nibble should be set to 1 to be output lines and 0 to be input lines (only used in SetSensorBitBang() ). The bits in the lower nibble should be set to 1 initially to be active lines.



ftStatus = FT_ListDevices(&numDevs,NULL,FT_LIST_NUMBER_ONLY);
printf("NumDev: %d\n",numDevs);


status = Ftdi::InitFtdi(devNum,&ftHandle,1,Mask);
printf("Dev0: %d\n",status);

UCHAR dataBit;

FT_GetBitMode(ftHandle, &dataBit);

printf("DataByte: %x\n",dataBit);

if (status==0)
	printf("Sensor 0 found and opened.\n");
else
{
	if (status == 1)
		printf("Failed to create device list.\n");
	else if (status == 2)
		printf("Sensor ID=%d not found.\n",devNum);
	else if (status == 3)
		printf("Sensor %d failed to open.\n",devNum);
	else if (status == 4)
		printf("Sensor %d failed to start in BitBang mode.\n",devNum);
	else
		printf("UNDEFINED ERROR!\n");

	printf("Ending program.\n");
	return (0);

}


/*
for (int a = 0; a < 3000; a++)
{
	
	data1 = PhotoSensor::GetSensorData(ftHandle);
	if (data1 == -1)
		printf("%5d : Invalid Sensor data\r",a);
	else
		printf("%5d : Sensor reading: %d \r",a,data1);
	
	Sleep(10);
}
*/



UCHAR bit;

for (int a = 0; a < 1000; a++)
{

	Mask = 0x4b;

	if (a == 0)
	{
		status = Ftdi::SetFtdiBitBang(ftHandle,Mask,3,0);
	
		if (status == 0)
			printf("Set Bits OK.\n");
		else if (status == -1)
			printf("Requested line is not an output line.\n");
	}
	else if (a == 250)
	{
		status = Ftdi::SetFtdiBitBang(ftHandle,Mask,4,1);
	
		if (status == 0)
			printf("Set Bits OK.\n");
		else if (status == -1)
			printf("Requested line is not an output line.\n");
	}
	else if (a == 500)
	{
		status = Ftdi::SetFtdiBitBang(ftHandle,Mask,3,1);
	
		if (status == 0)
			printf("Set Bits OK.\n");
		else if (status == -1)
			printf("Requested line is not an output line.\n");
	}



	bit = 4;  //this is the CTS line
	data1 = Ftdi::GetFtdiBitBang(ftHandle,bit);

	bit = 3;  //this is the RxD line
	data2 = Ftdi::GetFtdiBitBang(ftHandle,bit);

	bit = 2;  //this is the RxD line
	data3 = Ftdi::GetFtdiBitBang(ftHandle,bit);

	bit = 1;  //this is the RxD line
	data4 = Ftdi::GetFtdiBitBang(ftHandle,bit);

	//if (data1 == -1 || data2 == -1)
	//	printf("%5d : Invalid Sensor data.\r",a);
	//else
		printf("%5d :  %d  %d  %d  %d\r",a,data1,data2,data3,data4);

	Sleep(10);
}


printf("\n\n");




status = Ftdi::CloseFtdi(ftHandle,1);
if (status)
	printf("Sensor %d closed.\n",devNum);
else
	printf("Sensor %d failed to close.\n",devNum);


return 0;

}
