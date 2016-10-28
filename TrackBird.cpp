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

	TrackSYSCONFIG sysconfigstatus;

	//initialize general constants
	sysconfig->measureRate = SAMPRATE;
	sysconfig->reportRate = REPORTRATE;
	sysconfig->birdCount = BIRDCOUNT;
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
		sysconfig->groupID = 1;
		
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

		if (birdRS232WakeUp(sysconfig->groupID, FALSE, sysconfig->birdCount, sysconfig->SensorIDs, 115200,
			FOB_READ_TIMEOUT, FOB_WRITE_TIMEOUT, GMS_GROUP_MODE_NEVER))
		{

			std::cerr << "FOB System Initialized." << std::endl;

			//set up system configuration - must do this BEFORE starting to stream
			birdGetSystemConfig(sysconfig->groupID, &fob_sysconfig); //get defaults
			fob_sysconfig.dMeasurementRate = sysconfig->measureRate;  //edit sampling rate
			birdSetSystemConfig(sysconfig->groupID, &fob_sysconfig); //set sampling rate
			birdGetSystemConfig(sysconfig->groupID, &fob_sysconfig); //re-update saved values
			sysconfig->measureRate = fob_sysconfig.dMeasurementRate;
			std::cerr << "FOB Sampling Rate: " << sysconfig->measureRate << std::endl;
			
			for (int i = 1; i <= sysconfig->birdCount; i++)
			{
				//shut down the filtering on all channels
				BIRDDEVICECONFIG pdevcfg;
				birdGetDeviceConfig(sysconfig->groupID, i, &pdevcfg);  //may have to do this for each bird,which should be devices 1 through 4.
				std::cerr << "Pre ConfigSetup B" << i << ": " << std::hex << int(pdevcfg.bySetup) << std::dec;		

				if (sysconfig->filter_wide)
					fob_filter_all = fob_filter_all | fob_filter_wide;  //filters are active high, set proper bit to 1 to turn on.
				if (sysconfig->filter_narrow)
					fob_filter_all = fob_filter_all | fob_filter_narrow; //filters are active high, set proper bit to 1 to turn on.
				if (sysconfig->filter_dc > 0)
					fob_filter_all = fob_filter_all | fob_filter_dc;  //filters are active high, set proper bit to 1 to turn on.

				if (sysconfig->filter_dc == 0)
				{
				}

				pdevcfg.bySetup = (pdevcfg.bySetup & 0x00) | fob_filter_all;  //set the last three bits to be zero, which should turn off all the filters (and with 0x00). then, set filtering back on (set to one) as specified
				pdevcfg.byDataFormat = BDF_POSITIONMATRIX;
				birdSetDeviceConfig(sysconfig->groupID, i, &pdevcfg);
				birdGetDeviceConfig(sysconfig->groupID, i, &pdevcfg);
				std::cerr << "Pst ConfigSetup B" << i << ": " << std::hex << int(pdevcfg.bySetup) << std::dec << std::endl;
			}

			if (birdStartFrameStream(sysconfig->groupID))  //run in streaming mode
				return 1;
			else
				birdShutDown(sysconfig->groupID);

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

		sysconfig->datatype = DOUBLE_POSITION_MATRIX_TIME_Q;
		//sysconfig->reportRate = 3;

		SENSOR_CONFIGURATION pSensor[5];
		SYSTEM_CONFIGURATION trakSysConfig;

		//set up system (including sampling rate)
		errorCode = InitializeBIRDSystem();
		if(errorCode!=BIRD_ERROR_SUCCESS) 
		{
			std::cerr << "trakSTAR unable to initialize." << std::endl;
			didsetup = 0;
		}
		else
			std::cerr << "trakSTAR System Initialized." << std::endl;

		char errorbuffer[1024];
		char *perrbuff = &errorbuffer[0];

		errorCode = SetSystemParameter(MEASUREMENT_RATE, &sysconfig->measureRate, sizeof(sysconfig->measureRate));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			GetErrorText(errorCode, perrbuff, sizeof(errorbuffer), SIMPLE_MESSAGE);
			std::cerr << "trakSTAR Units MeasurementRate Error: " << errorbuffer << std::endl;  // << std::endl;

		}
		GetSystemParameter(MEASUREMENT_RATE, &sysconfig->measureRate, sizeof(sysconfig->measureRate));
		std::cerr << "trakSTAR Sampling Rate: " << sysconfig->measureRate <<std::endl;

		errorCode = SetSystemParameter(METRIC, &sysconfig->metric, sizeof(sysconfig->metric));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			GetErrorText(errorCode, perrbuff, sizeof(errorbuffer), SIMPLE_MESSAGE);
			std::cerr << "trakSTAR Units Metric Error: " << errorbuffer << std::endl;  // << std::endl;
		}
		GetSystemParameter(METRIC, &sysconfig->metric, sizeof(sysconfig->metric));
		std::cerr << "trakSTAR Units Metric: " << sysconfig->metric <<std::endl;

		GetBIRDSystemConfiguration(&trakSysConfig);

		
		//this only applies for STREAM, not for Asynchronous sampling; one out of every reportRate samples is returned
		errorCode = SetSystemParameter(REPORT_RATE, &sysconfig->reportRate, sizeof(sysconfig->reportRate));
		/*
		if(errorCode != BIRD_ERROR_SUCCESS)
		{	
			char errBuf[1024];
			char *pErrBuf = &errBuf[0];
			GetErrorText(errorCode,pErrBuf,sizeof(errBuf),SIMPLE_MESSAGE);
			std::cerr << "trakSTAR ReportRate Error: " << errBuff << std::endl;
		}
		else
		*/
		GetSystemParameter(REPORT_RATE, &sysconfig->reportRate, sizeof(sysconfig->reportRate));
		std::cerr << "trakSTAR Report Rate: " << std::hex << sysconfig->reportRate << std::dec << std::endl;


		//set up sensors (and set filtering parameters)
		for (i = 0; i<trakSysConfig.numberSensors; i++)
		{
			errorCode = GetSensorConfiguration(i, &pSensor[i]);

			errorCode = SetSensorParameter(i, DATA_FORMAT, &sysconfig->datatype, sizeof(sysconfig->datatype));

			//if(errorCode!=BIRD_ERROR_SUCCESS) errorHandler(errorCode);
			if(pSensor[i].attached)
			{
				sysconfig->SensorIDs[i + 1] = i; //record which sensors are attached
				NSensors++;

				errorCode = GetSensorParameter(i, FILTER_AC_WIDE_NOTCH, &sysconfigstatus.filter_wide, sizeof(sysconfigstatus.filter_wide));
				errorCode = GetSensorParameter(i, FILTER_AC_NARROW_NOTCH, &sysconfigstatus.filter_narrow, sizeof(sysconfigstatus.filter_narrow));
				errorCode = GetSensorParameter(i, FILTER_DC_ADAPTIVE, &sysconfigstatus.filter_dc, sizeof(sysconfigstatus.filter_dc));
				errorCode = GetSensorParameter(i, FILTER_ALPHA_PARAMETERS, &sysconfig->alpha_parameters, sizeof(sysconfig->alpha_parameters));

				std::cerr << "trakSTAR Original Filter Status: " << sysconfigstatus.filter_wide << " " 
															 	 << sysconfigstatus.filter_narrow << " " 
																 << sysconfigstatus.filter_dc << " " 
																 << sysconfig->alpha_parameters.alphaOn << " "
																 <<std::endl;
				
				//if the adaptive DC filter is set to zero, the alphaOn parameter must also be turned off to fully shut off the filter.
				if (sysconfig->filter_dc <= 0.0f)
					sysconfig->alpha_parameters.alphaOn = false;
				else
					sysconfig->alpha_parameters.alphaOn = true;

				//set status of all the filters
				errorCode = SetSensorParameter(i, FILTER_AC_WIDE_NOTCH,&sysconfig->filter_wide, sizeof(sysconfig->filter_wide));
				errorCode = SetSensorParameter(i, FILTER_AC_NARROW_NOTCH,&sysconfig->filter_narrow, sizeof(sysconfig->filter_narrow));
				errorCode = SetSensorParameter(i, FILTER_DC_ADAPTIVE,&sysconfig->filter_dc, sizeof(sysconfig->filter_dc));
				errorCode = SetSensorParameter(i, FILTER_ALPHA_PARAMETERS,&sysconfig->alpha_parameters, sizeof(sysconfig->alpha_parameters));

				errorCode = GetSensorParameter(i, FILTER_AC_WIDE_NOTCH,&sysconfig->filter_wide, sizeof(sysconfig->filter_wide));
				errorCode = GetSensorParameter(i, FILTER_AC_NARROW_NOTCH,&sysconfig->filter_narrow, sizeof(sysconfig->filter_narrow));
				errorCode = GetSensorParameter(i, FILTER_DC_ADAPTIVE,&sysconfig->filter_dc, sizeof(sysconfig->filter_dc));
				errorCode = GetSensorParameter(i, FILTER_ALPHA_PARAMETERS,&sysconfig->alpha_parameters, sizeof(sysconfig->alpha_parameters));

				std::cerr << "trakSTAR Filter Status: " << sysconfig->filter_wide << " " 
														<< sysconfig->filter_narrow << " " 
														<< sysconfig->filter_dc << " " 
														<< sysconfig->alpha_parameters.alphaOn << " "
													    <<std::endl;

			}
			else
				sysconfig->SensorIDs[i + 1] = 5000;

		}//end for setup sensors
		sysconfig->nBirdsActive = NSensors;

		std::cerr << "Sensors Active: (" << sysconfig->nBirdsActive << ":"
										 << sysconfig->birdCount << ") "
										 << sysconfig->SensorIDs[0] << " "
										 << sysconfig->SensorIDs[1] << " "
										 << sysconfig->SensorIDs[2] << " "
										 << sysconfig->SensorIDs[3] << " "
										 << sysconfig->SensorIDs[4] << " "
										 <<std::endl;

		//set up the transmitter.  there is only 1.
		USHORT t_id = 0;
		errorCode = SetSystemParameter(SELECT_TRANSMITTER, &t_id, sizeof(t_id));
		if(errorCode != BIRD_ERROR_SUCCESS)
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




int TrackBird::GetUpdatedSample(TrackSYSCONFIG *sysconfig, TrackDATAFRAME DataBirdFrame[])
{

	int sampleUpdated = 0;
	bool UpdatedSamples[BIRDCOUNT + 1] = {false};
	int j, k, m;

	//FOB system, poll for sample
	if (sysconfig->trackType == 0)
	{
		BIRDFRAME frame;
		BIRDREADING bird_data;

		if (birdFrameReady(sysconfig->groupID))  //poll to see if a sample is available.  note, all the sensors get updated at the same time.
		{
			birdGetFrame(sysconfig->groupID, &frame);

			for (j = 1; j <= sysconfig->birdCount; j++)
			{
				if (sysconfig->SensorIDs[j] > 1000)
				{
					DataBirdFrame[j].ValidInput = 0;
					continue;
				}

				sampleUpdated++;
				UpdatedSamples[j] = true;
				DataBirdFrame[j].ValidInput = 1;

				bird_data = frame.reading[j];
				if (birds_start == 0)
					birds_start = frame.dwTime/1000.0f;

				//Note, the way that the tracker is mounted requires a change of coordinate frames.  
				//Tracker X = data Z
				//Tracker Y = data X
				//Tracker Z = data Y
				//For angle, we will record just the angle matrix rather than the azimuth/elevation/roll, to avoid the reference-frame rotation problem
				DataBirdFrame[j].time = double(frame.dwTime)/1000.0f;  //convert time in msec to time in sec.
				DataBirdFrame[j].etime = double(frame.dwTime)-birds_start;
				DataBirdFrame[j].x = bird_data.position.nY;
				DataBirdFrame[j].y = bird_data.position.nZ;
				DataBirdFrame[j].z = bird_data.position.nX;

				for (k = 0; k < 3; k++)  //record the angle matrix
					for (m = 0; m < 3; m++)
					{
						DataBirdFrame[j].anglematrix[k][m] = bird_data.matrix.n[k][m];
					}

				DataBirdFrame[j].quality = NULL;

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
				DataBirdFrame[j].x = (DataBirdFrame[j].x * 0.9144f / 32768.f);
				DataBirdFrame[j].y = (DataBirdFrame[j].y * 0.9144f / 32768.f);
				DataBirdFrame[j].z = (DataBirdFrame[j].z * 0.9144f / 32768.f);

				//rotate axes to align display and flock of birds
				double tmpx, tmpy;
				tmpx = DataBirdFrame[j].x;
				tmpy = DataBirdFrame[j].y;
				DataBirdFrame[j].x = tmpx*cos(CALxyROTANG) - tmpy*sin(CALxyROTANG);
				DataBirdFrame[j].y = tmpx*sin(CALxyROTANG) + tmpy*cos(CALxyROTANG);
				
				DataBirdFrame[j].x += CALxOFFSET;
				DataBirdFrame[j].y += CALyOFFSET;

				//rotations: note, for now, these have NOT been transformed into the appropriate coordinate frame.

				for (k = 0; k < 3; k++)  //record the angle matrix
					for (m = 0; m < 3; m++)
					{
						DataBirdFrame[j].anglematrix[k][m] = (DataBirdFrame[j].anglematrix[k][m] / 32768.f) * atan(1.0f) * 4;
					}

			}

		}

	}
	else  //trakSTAR system, poll for sample
	{
		DOUBLE_POSITION_MATRIX_TIME_Q_RECORD bird_data[4], *pbird_data = &bird_data[0];
		int errorCode;
		
		//We request data from all four sensors (available and disconnected) to be returned. We can do this synchronously using the REPORT_RATE value, or asychronously.
		//We will use the REPORTRATE value as a flag to indicate whether to use synchronous or asynchronous data requests. Note that if you stream synchronously and 
		//  samples are returned faster than they can be read, it is unclear if you will pull the most recent sample or an older, buffered one with this call. 
		//  Alternatively, using asynchronous recording means you always get the most updated sample, but you might occasionally drop samples if they are returned too rapidly
		if (REPORTRATE > 1)
			errorCode = GetSynchronousRecord(ALL_SENSORS, pbird_data, sizeof(bird_data[0])*4);
		else
			errorCode = GetAsynchronousRecord(ALL_SENSORS, pbird_data, sizeof(bird_data[0])*4);

		for (j = 1; j <= sysconfig->birdCount; j++)
		{

			if (sysconfig->SensorIDs[j] > 1000)
			{
				DataBirdFrame[j].ValidInput = 0;
				continue;
			}

			//errorCode = GetAsynchronousRecord(sysconfig->SensorIDs[j], &bird_data, sizeof(bird_data));
			if (fabs(bird_data[j-1].time - DataBirdFrame[j].time) > 1e-4 )  //if this is a new sample, save it -- based on the time stamp. note, the danger is that some trackers may be updated and others not, depending on the timing of these calls
			{
				if (birds_start == 0)
					birds_start = bird_data[j-1].time;

				UpdatedSamples[j] = true;
				DataBirdFrame[j].ValidInput = 1;

				sampleUpdated++;

				//Note, the way that the tracker is mounted requires a change of coordinate frames.  
				//Tracker X = data Z
				//Tracker Y = data X
				//Tracker Z = data Y
				//For angle, we will record just the angle matrix rather than the azimuth/elevation/roll, to avoid the reference-frame rotation problem
				DataBirdFrame[j].time = bird_data[j-1].time;
				DataBirdFrame[j].etime = bird_data[j-1].time-birds_start;
				DataBirdFrame[j].x = bird_data[j-1].y;
				DataBirdFrame[j].y = -bird_data[j-1].z;  //note, for the minireach setup this sign must be inverted since the transmitter is flipped.
				DataBirdFrame[j].z = bird_data[j-1].x;

				for (k = 0; k < 3; k++)  //record the angle matrix
					for (m = 0; m < 3; m++)
					{
						DataBirdFrame[j].anglematrix[k][m] = bird_data[j-1].s[k][m];
					}

				DataBirdFrame[j].quality = bird_data[j-1].quality;

				/*calibrate the data.  
				 *  For trakSTAR, the data already arrives calibrated in mm, so we just have to convert to meters.
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
				DataBirdFrame[j].x = (DataBirdFrame[j].x)/1000;
				DataBirdFrame[j].y = (DataBirdFrame[j].y)/1000; 
				DataBirdFrame[j].z = (DataBirdFrame[j].z)/1000;

				//rotate axes to align display and flock of birds
				double tmpx, tmpy;
				tmpx = DataBirdFrame[j].x;
				tmpy = DataBirdFrame[j].y;
				DataBirdFrame[j].x = tmpx*cos(CALxyROTANG) - tmpy*sin(CALxyROTANG);
				DataBirdFrame[j].y = tmpx*sin(CALxyROTANG) + tmpy*cos(CALxyROTANG);

				DataBirdFrame[j].x += CALxOFFSET;
				DataBirdFrame[j].y += CALyOFFSET;

				//rotations: note, for now, the rotation matrix remains uncalibrated.
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
		birdStopFrameStream(sysconfig->groupID);  //stop streaming data frames
		birdShutDown(sysconfig->groupID);

	}
	else  //trakSTAR system
	{
		int t_id = -1;
		errorCode = SetSystemParameter(SELECT_TRANSMITTER, &t_id, sizeof(t_id));  //Shut off the transmitter
		if (errorCode != BIRD_ERROR_SUCCESS)
			shutoff = false;
		errorCode = CloseBIRDSystem();
		if (errorCode != BIRD_ERROR_SUCCESS)
			shutoff = false;

	}

	return(shutoff);

}