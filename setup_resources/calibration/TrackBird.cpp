#include <cmath>
#include <math.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>

#include <windows.h>

#include "TrackBird.h"




int TrackBird::InitializeBird(TrackSYSCONFIG *sysconfig)
{

	int temp;

	//initialize general constants
	sysconfig->measureRate = SAMPRATE;
	sysconfig->BirdCount = BIRDCOUNT;
	sysconfig->trackType = TRACKTYPE;
	sysconfig->filter_wide = FILTER_WIDE;
	sysconfig->filter_narrow = FILTER_NARROW;
	sysconfig->filter_dc = FILTER_DC;
	sysconfig->SensorIDs[0] = 0;  //this is the mouse.
	sysconfig->SensorIDs[1] = 5000;
	sysconfig->SensorIDs[2] = 5000;
	sysconfig->SensorIDs[3] = 5000;
	sysconfig->SensorIDs[4] = 5000;

	std::cerr << "Tracker Type: " << sysconfig->trackType << std::endl;

	//FOB initialization code
	if (sysconfig->trackType == 0)
	{
		//initialize constants
		sysconfig->GroupID = 1;
		
		//these need to match the ports that Windows has detected (see notes, AW for how to set up)
		sysconfig->SensorIDs[1] = 5;
		sysconfig->SensorIDs[2] = 6;
		sysconfig->SensorIDs[3] = 7;  //this is the right hand!
		sysconfig->SensorIDs[4] = 8;

		BIRDSYSTEMCONFIG fob_sysconfig;
		BYTE fob_filter_narrow = 0x04;
		BYTE fob_filter_wide = 0x02;
		BYTE fob_filter_dc = 0x01;
		BYTE fob_filter_all = 0x00;  //all bits except the last three should be set to zero

		if (birdRS232WakeUp(sysconfig->GroupID, FALSE, sysconfig->BirdCount, sysconfig->SensorIDs, 115200,
			FOB_READ_TIMEOUT, FOB_WRITE_TIMEOUT, GMS_GROUP_MODE_NEVER))
		{

			std::cerr << "FOB System Initialized." << std::endl;

			//set up system configuration - must do this BEFORE starting to stream
			birdGetSystemConfig(sysconfig->GroupID,&fob_sysconfig); //get defaults
			fob_sysconfig.dMeasurementRate = sysconfig->measureRate;  //edit sampling rate
			temp = birdSetSystemConfig(sysconfig->GroupID,&fob_sysconfig); //set sampling rate
			//std::cerr << birdGetErrorMessage() << std::endl;
			birdGetSystemConfig(sysconfig->GroupID,&fob_sysconfig); //re-update saved values
			sysconfig->measureRate = fob_sysconfig.dMeasurementRate;
			std::cerr << "FOB Sampling Rate: " << sysconfig->measureRate << std::endl;
			
			for (int i = 1; i <= sysconfig->BirdCount; i++)
			{
				//shut down the filtering on all channels
				BIRDDEVICECONFIG pdevcfg;
				birdGetDeviceConfig(sysconfig->GroupID, i, &pdevcfg);  //may have to do this for each bird,which should be devices 1 through 4.
				std::cerr << "Pre ConfigSetup B" << i << ": " << std::hex << int(pdevcfg.bySetup) << std::dec;
				//std::cerr << "Pre Data Format B" << i << ": " << pdevcfg.byDataFormat;
				//printf("Pre Data Format: %d\n",pdevcfg.byDataFormat);
				
				if (sysconfig->filter_wide)
					fob_filter_all = fob_filter_all | fob_filter_wide;  //filters are active high, set proper bit to 1 to turn on.
				if (sysconfig->filter_narrow)
					fob_filter_all = fob_filter_all | fob_filter_narrow; //filters are active high, set proper bit to 1 to turn on.
				if (sysconfig->filter_dc > 0)
					fob_filter_all = fob_filter_all | fob_filter_dc;  //filters are active high, set proper bit to 1 to turn on.

				pdevcfg.bySetup = (pdevcfg.bySetup & 0x00) | fob_filter_all;  //set the last three bits to be zero, which should turn off all the filters (and with 0x00). then, set filtering back on (set to one) as specified
				pdevcfg.byDataFormat = BDF_POSITIONMATRIX;
				birdSetDeviceConfig(sysconfig->GroupID, i, &pdevcfg);
				birdGetDeviceConfig(sysconfig->GroupID, i, &pdevcfg);
				std::cerr << "  Pst ConfigSetup B" << i << ": " << pdevcfg.bySetup << std::endl;
				//printf("Post Data Format: %d\n",pdevcfg.byDataFormat);
			}

			if (birdStartFrameStream(sysconfig->GroupID))  //run in streaming mode
			//if (birdStartSingleFrame(sysconfig->GroupID))  //we want to run in single-frame mode, not streaming -- this will help avoid weird delays since we are polling the FOB
				return 1;
			else
				birdShutDown(sysconfig->GroupID);

		} // end wakeup birds
		else
		{
			std::cerr << "Birds did not wake up." << std::endl;
		}
		
	}
	else //trakSTAR initialization code
	{

		int errorCode;
		int didsetup = 2;
		int NSensors = 0;

		//set system parameters
		sysconfig->metric = true;
		
		int i;
		int j;

		//sysconfig->datatype = DOUBLE_POSITION_ANGLES_TIME_Q;
		sysconfig->datatype = DOUBLE_POSITION_MATRIX_TIME_Q;
		sysconfig->reportRate = 3;

		SENSOR_CONFIGURATION pSensor[5];
		SYSTEM_CONFIGURATION trakSysConfig;

		//set up system (including sampling rate)
		//printf("Initializing ATC3DG system...\n");
		errorCode = InitializeBIRDSystem();
		if(errorCode!=BIRD_ERROR_SUCCESS) 
		{
			//errorHandler(errorCode);
			std::cerr << "trakSTAR unable to initialize." << std::endl;
			didsetup = 0;
		}
		else
			std::cerr << "trakSTAR System Initialized." << std::endl;

		errorCode = SetSystemParameter(MEASUREMENT_RATE, &sysconfig->measureRate, sizeof(sysconfig->measureRate));
		GetSystemParameter(MEASUREMENT_RATE, &sysconfig->measureRate, sizeof(sysconfig->measureRate));
		std::cerr << "trakSTAR Sampling Rate: " << sysconfig->measureRate <<std::endl;
		//if(errorCode!=BIRD_ERROR_SUCCESS) 
			//errorHandler(errorCode);

		errorCode = SetSystemParameter(METRIC, &sysconfig->metric, sizeof(sysconfig->metric));
		GetSystemParameter(METRIC, &sysconfig->metric, sizeof(sysconfig->metric));
		std::cerr << "trakSTAR Units Metric: " << sysconfig->metric <<std::endl;

		GetBIRDSystemConfiguration(&trakSysConfig);

		/*
		//this only applies for STREAM, not for Asynchronous sampling as we are doing
		errorCode = SetSystemParameter(REPORT_RATE, &reportrate, sizeof(reportrate));
		if(errorCode != BIRD_ERROR_SUCCESS)
		errorHandler(errorCode);
		else
		GetSystemParameter(REPORT_RATE, &reportrate, sizeof(reportrate));
		*/

		//set up sensors (and set filtering parameters)
		//for (i = 0; i<trakSysConfig.numberSensors; i++)
		for (i = 0; i<trakSysConfig.numberSensors; i++)
		{
			errorCode = GetSensorConfiguration(i, &pSensor[i]);

			//if(errorCode!=BIRD_ERROR_SUCCESS) errorHandler(errorCode);
			if(pSensor[i].attached)
			{
				//sysconfig->SensorIDs[++NSensors] = i; //record which sensors are attached
				sysconfig->SensorIDs[i+1] = i; //record which sensors are attached
				NSensors++;

				//set status of all the filters
				errorCode = SetSensorParameter(i,FILTER_AC_WIDE_NOTCH,&sysconfig->filter_wide, sizeof(sysconfig->filter_wide));
				errorCode = SetSensorParameter(i,FILTER_AC_NARROW_NOTCH,&sysconfig->filter_narrow, sizeof(sysconfig->filter_narrow));
				errorCode = SetSensorParameter(i,FILTER_DC_ADAPTIVE,&sysconfig->filter_dc, sizeof(sysconfig->filter_dc));

				errorCode = GetSensorParameter(i,FILTER_AC_WIDE_NOTCH,&sysconfig->filter_wide, sizeof(sysconfig->filter_wide));
				errorCode = GetSensorParameter(i,FILTER_AC_NARROW_NOTCH,&sysconfig->filter_narrow, sizeof(sysconfig->filter_narrow));
				errorCode = GetSensorParameter(i,FILTER_DC_ADAPTIVE,&sysconfig->filter_dc, sizeof(sysconfig->filter_dc));

				std::cerr << "trakSTAR Filter Status: " << sysconfig->filter_wide << " " 
														<< sysconfig->filter_narrow << " " 
														<< sysconfig->filter_dc << " " 
													    <<std::endl;

				//printf("Sensor %d: Filter status: %d %d %.2f\n", i,sysconfig->filter_wide,sysconfig->filter_narrow,sysconfig->filter_dc);

				errorCode = SetSensorParameter(i,DATA_FORMAT,&sysconfig->datatype, sizeof(sysconfig->datatype));
			}
			else
				sysconfig->SensorIDs[i+1] = 5000;

		}//end for setup sensors
		//sysconfig->BirdCount = NSensors;  //update the Bird Count with the true number of attached sensors
		sysconfig->NBirdsActive = NSensors;

		std::cerr << "Sensors Active: (" << sysconfig->NBirdsActive << ":"
										 << sysconfig->BirdCount << ") "
										 << sysconfig->SensorIDs[0] << " "
										 << sysconfig->SensorIDs[1] << " "
										 << sysconfig->SensorIDs[2] << " "
										 << sysconfig->SensorIDs[3] << " "
										 << sysconfig->SensorIDs[4] << " "
										 <<std::endl;

		//set up the transmitter.  there is only 1.
		USHORT t_id = 0;
		errorCode = SetSystemParameter(SELECT_TRANSMITTER, &t_id, sizeof(t_id));
		if(errorCode!=BIRD_ERROR_SUCCESS)
		{
			std::cerr << "trakSTAR Transmitter OFF!" << std::endl;
			didsetup = 0;
		}
		else
			std::cerr << "trakSTAR Transmitter On." << std::endl;


		if (didsetup > 0)
			return didsetup;
		else
		{
			//something in the set-up failed, shut down the system.
			t_id = -1;
			SetSystemParameter(SELECT_TRANSMITTER, &t_id, sizeof(t_id));
			CloseBIRDSystem();
		}
		

	} //end TRACKTYPE

	return 0;  //if setup was unsuccessful, report it!
}




int TrackBird::GetUpdatedSample(TrackSYSCONFIG *sysconfig,TrackDATAFRAME *DataBirdFrame)
{

	int sampleUpdated = 0;
	bool UpdatedSamples[BIRDCOUNT+1] = {false};
	int j,k,m;

	//FOB system, poll for sample
	if (sysconfig->trackType == 0)
	{
		BIRDFRAME frame;
		BIRDREADING bird_data;

		if (birdFrameReady(sysconfig->GroupID))  //poll to see if a sample is available.  note, all the sensors get updated at the same time.
		{
			//birdGetMostRecentFrame(sysconfig->GroupID, &frame);  //use this when in STREAM mode
			birdGetFrame(sysconfig->GroupID, &frame);

			for (j = 1; j <= sysconfig->BirdCount; j++)
			{
				if (sysconfig->SensorIDs[j] > 1000)
					continue;

				sampleUpdated++;
				UpdatedSamples[j] = true;

				bird_data = frame.reading[j];
				if (birds_start == 0)
					birds_start = frame.dwTime/1000.0f;

				//Note, the way that the tracker is mounted requires a change of coordinate frames.  
				//Tracker X = data Z
				//Tracker Y = data X
				//Tracker Z = data Y
				//Tracker Aximuth (rotation from XZ plane) = data elevation?
				//Tracker Elevation (rotation from XY plane)= data azimuth?
				//Tracker Roll (rotation from about Z axis)= data roll?
				DataBirdFrame->time[j] = frame.dwTime;  //convert time in msec to time in sec.
				DataBirdFrame->etime[j] = frame.dwTime-birds_start;
				DataBirdFrame->x[j] = bird_data.position.nY;
				DataBirdFrame->y[j] = bird_data.position.nZ;
				DataBirdFrame->z[j] = bird_data.position.nX;
				//DataBirdFrame->azimuth[j] = bird_data.angles.nElevation;
				//DataBirdFrame->elevation[j] = bird_data.angles.nAzimuth;
				//DataBirdFrame->roll[j] = bird_data.angles.nRoll;
				for (k = 0; k < 3; k++)  //record the angle matrix
					for (m = 0; m < 3; m++)
					{
						DataBirdFrame->anglematrix[j][k][m] = bird_data.matrix.n[k][m];
					}

				DataBirdFrame->quality[j] = NULL;

				/*calibrate the data.  
				 *  For FOB, data is in arbitrary units so we have to convert to meters using a constant 
				 *  (0.9144f/32768.f) first - this comes from (36 in = 32768).
				 *  To convert orientations to radians, multiply by atan(1) * 4 / 32768 (180 deg = 32767). 
				 *  where we use atan(1)*4 to get to pi radians (or, 180 deg).
				 *  
				 *  Keep in mind that the coordinate system used by the returned Flock of
				 *  Birds data is different than the one used to output to the screen. "Up"
				 *  on the screen is the -z axis for the Flock of Birds and +y for OpenGL.
				 *  "Right" on the screen is the +y axis for the Flock of Birds and +x for
				 *  OpenGL.
				 *  
				 *  We then subtract off the offset.
				 *  
				 */
				DataBirdFrame->x[j] = (DataBirdFrame->x[j] * 0.9144f / 32768.f) + CALxOFFSET;
				DataBirdFrame->y[j] = (DataBirdFrame->y[j] * 0.9144f / 32768.f) + CALyOFFSET;
				DataBirdFrame->z[j] = (DataBirdFrame->z[j] * 0.9144f / 32768.f);  //offset not measured for Z, as we don't use this.

				//rotate axes to align display and flock of birds
				double tmpx, tmpy;
				tmpx = DataBirdFrame->x[j];
				tmpy = DataBirdFrame->y[j];
				DataBirdFrame->x[j] = tmpx*cos(CALxyROTANG) - tmpy*sin(CALxyROTANG);
				DataBirdFrame->y[j] = tmpx*sin(CALxyROTANG) + tmpy*cos(CALxyROTANG);

				//rotations: note, for now, these are uncalibrated! See AW for calibration data.

				//DataBirdFrame->roll[j] = -(DataBirdFrame->roll[j] / 32768.f) * atan(1.0f) * 4; //display_yaw = bird_roll (rot about x axis)
				//input.pitch = -(bird_data->angles.nAzimuth / 32768.f) * atan(1.0f) * 4; //display_pitch = bird_azimuth/yaw (rot about z axis)
				//input.roll = -(bird_data->angles.nElevation / 32768.f) * atan(1.0f) * 4; //display_roll = bird_elevation/pitch (rot about y axis)
				for (k = 0; k < 3; k++)  //record the angle matrix
					for (m = 0; m < 3; m++)
					{
						DataBirdFrame->anglematrix[j][k][m] = (DataBirdFrame->anglematrix[j][k][m] / 32768.f) * atan(1.0f) * 4;
					}


			}

		}

	}
	else  //trakSTAR system, poll for sample
	{
		//OUBLE_POSITION_ANGLES_TIME_Q_RECORD bird_data;
		DOUBLE_POSITION_MATRIX_TIME_Q_RECORD bird_data;
		int errorCode;
		
		//the first SensorID is the mouse, so we look for valid bird sensor IDs which start at array position 1.
		for (j = 1; j <= sysconfig->BirdCount; j++)
		{

			if (sysconfig->SensorIDs[j] > 1000)
				continue;

			errorCode = GetAsynchronousRecord(sysconfig->SensorIDs[j], &bird_data, sizeof(bird_data));
			if (bird_data.time != DataBirdFrame->time[j])  //if this is a new sample, save it -- based on the time stamp. if the time stamp is a funcion-call time and not a sample time we will get repeated records
			{
				if (birds_start == 0)
					birds_start = bird_data.time;

				UpdatedSamples[j] = true;

				sampleUpdated++;

				//Note, the way that the tracker is mounted requires a change of coordinate frames.  
				//Tracker X = data Z
				//Tracker Y = data X
				//Tracker Z = data Y
				//Tracker Aximuth (rotation from XZ plane) = data elevation?
				//Tracker Elevation (rotation from XY plane)= data azimuth?
				//Tracker Roll (rotation from about Z axis)= data roll?
				DataBirdFrame->time[j] = bird_data.time;
				DataBirdFrame->etime[j] = bird_data.time-birds_start;
				DataBirdFrame->x[j] = bird_data.y;
				DataBirdFrame->y[j] = bird_data.z;
				DataBirdFrame->z[j] = bird_data.x;
				//DataBirdFrame->azimuth[j] = bird_data.e;
				//DataBirdFrame->elevation[j] = bird_data.a;
				//DataBirdFrame->roll[j] = bird_data.r;
				for (k = 0; k < 3; k++)  //record the angle matrix
					for (m = 0; m < 3; m++)
					{
						DataBirdFrame->anglematrix[j][k][m] = bird_data.s[k][m];
					}

				DataBirdFrame->quality[j] = bird_data.quality;

				/*calibrate the data.  
				 *  For trakSTAR, the data already arrives calibrated in mm (cm?), so we just have to convert to meters.
				 *  The orientations are provided in degrees, so we have to convert to radians by multiplying by atan(1) * 4 / 180. 
				 *  where we use atan(1)*4 to get to pi radians (or, 180 deg).
				 *  
				 *  Keep in mind that the coordinate system used by the returned Flock of
				 *  Birds data is different than the one used to output to the screen. "Up"
				 *  on the screen is the -z axis for the Flock of Birds and +y for OpenGL.
				 *  "Right" on the screen is the +y axis for the Flock of Birds and +x for
				 *  OpenGL.
				 *  
				 *  We then subtract off the offset.
				 */
				DataBirdFrame->x[j] = (DataBirdFrame->x[j])/100 + CALxOFFSET;
				DataBirdFrame->y[j] = (DataBirdFrame->y[j])/100 + CALyOFFSET;
				DataBirdFrame->z[j] = (DataBirdFrame->z[j])/100;

				//rotate axes to align display and flock of birds
				double tmpx, tmpy;
				tmpx = DataBirdFrame->x[j];
				tmpy = DataBirdFrame->y[j];
				DataBirdFrame->x[j] = tmpx*cos(CALxyROTANG) - tmpy*sin(CALxyROTANG);
				DataBirdFrame->y[j] = tmpx*sin(CALxyROTANG) + tmpy*cos(CALxyROTANG);

				//rotations: note, for now, these are uncalibrated! See AW for calibration data.
				//DataBirdFrame->roll[j] = -DataBirdFrame->roll[j] * (atan(1.0f) * 4)/180; //display_yaw = bird_roll (rot about x axis)

			}
		}

	} //end trackType


	return(sampleUpdated);

}




bool TrackBird::ShutDownBird(TrackSYSCONFIG *sysconfig)
{

	bool shutoff = true;
	int errorCode;

	//FOB system
	if (sysconfig->trackType == 0)
	{
		birdStopFrameStream(sysconfig->GroupID);  //stop streaming data frames
		birdShutDown(sysconfig->GroupID);

	}
	else  //trakSTAR system
	{
		int t_id = -1;
		errorCode = SetSystemParameter(SELECT_TRANSMITTER, &t_id, sizeof(t_id));  //Shut off the transmitter
		if (errorCode!=BIRD_ERROR_SUCCESS)
			shutoff = false;
		errorCode = CloseBIRDSystem();
		if (errorCode!=BIRD_ERROR_SUCCESS)
			shutoff = false;

	}

	return(shutoff);

}