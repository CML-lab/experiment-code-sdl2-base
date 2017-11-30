#include "TrackCodamotion.h"

int TrackCoda::InitializeCoda()
{
	std::cerr << std::endl << "Starting CODA Initialization..." << std::endl;

	int ch;
	

	DWORD iServer = SERVER;


	//*** Connect to Coda Server ***


	if(true)  //autodiscover the server
	{
		// attempt auto-discover server
		CodaSysConfig.cl.doAutoDiscoverServer(CodaSysConfig.discover);

		// need at least one server to connect to
		if(CodaSysConfig.discover.dwNumServers == 0)
		{
			std::cerr << "ERROR: no RTNet Servers found using auto-discovery." << std::endl << "   Start the RTNetServer on the Codamotion host computer." << std::endl;
			return(1);
		}

		DWORD ip1,ip2,ip3,ip4;

		
		// list all servers
		std::cerr << "Found " << CodaSysConfig.discover.dwNumServers << "Codamotion Realtime Network servers:" << std::endl;
		for(DWORD iserver = 0; iserver < CodaSysConfig.discover.dwNumServers; iserver++)
		{
			ip1 = (CodaSysConfig.discover.server[iserver].ip & 0xFF000000) >> 24;
			ip2 = (CodaSysConfig.discover.server[iserver].ip & 0x00FF0000) >> 16;
			ip3 = (CodaSysConfig.discover.server[iserver].ip & 0x0000FF00) >>  8;
			ip4 = (CodaSysConfig.discover.server[iserver].ip & 0x000000FF);

			/*
			std::cerr << "   [" << iserver << "]: " << CodaSysConfig.discover.server[iserver].hostname 
				      << " IP: " << ip1 << "." << ip2 << "." << ip3 << "." << ip4 
					  << " port: " << CodaSysConfig.discover.server[iserver].port << std::endl;
			*/
			fprintf(stderr, "[%u]: %s IP: %u.%u.%u.%u port: %u\n",
			  iserver,
			  CodaSysConfig.discover.server[iserver].hostname,
			  (CodaSysConfig.discover.server[iserver].ip & 0xFF000000) >> 24,
			  (CodaSysConfig.discover.server[iserver].ip & 0x00FF0000) >> 16,
			  (CodaSysConfig.discover.server[iserver].ip & 0x0000FF00) >>  8,
			  (CodaSysConfig.discover.server[iserver].ip & 0x000000FF)      ,
			  CodaSysConfig.discover.server[iserver].port);
		}
		std::cerr << "   USING: server " << SERVER << std::endl;

	}
	else //manually specify the server
	{
		iServer = 0;
		CodaSysConfig.discover.dwNumServers = 1;
		// input ip address:
		// 10.0.0.111 = 0x0A00006F
		// MossRehab = 169.254.162.120 = 0xA9FEA278
		CodaSysConfig.discover.server[0].ip = 0xA9FEA278;
		// input port:
		CodaSysConfig.discover.server[0].port = (unsigned)10111;
	}


	// connect to selected server
	CodaSysConfig.cl.connect(CodaSysConfig.discover.server[iServer].ip, CodaSysConfig.discover.server[iServer].port);
	
	// get hardware config list
	codaRTNet::HWConfigEnum configs;
	CodaSysConfig.cl.enumerateHWConfig(configs);
	
	// print config names
	std::cerr << "Found " << configs.dwNumConfig << " hardware configurations:" << std::endl;
	for(DWORD iconfig = 0; iconfig < configs.dwNumConfig; iconfig++)
		//std::cerr << "   [" << iconfig << "] address: " << configs.config[iconfig].dwAddressHandle << " name: " << configs.config[iconfig].strName << std::endl; 
		fwprintf(stderr, L"  [%u] address: %08X  name: %s\n", 
			iconfig, configs.config[iconfig].dwAddressHandle, configs.config[iconfig].strName);
	
	// must have at least one config set up
	if(configs.dwNumConfig == 0)
	{
		std::cerr << "ERROR: no hardware configurations are available" << std::endl;
		return(2);
	}



	//*** configuration setup ***

	//open the configuration file to get all necessary parameters
	std::ifstream cfile(CONFIGFILE);
	if (!cfile)
	{
		std::cerr << "Cannot open config file." << std::endl;
		return(3);
	}

	char tmpline[100] = "";
	char configname[50] = "";
	char tmpname[50];


	//read the name of the hardware configuration to use
	cfile.getline(configname,sizeof(configname),'\n');  //get the first line of the file, which is the name of the configuration
	std::string t(configname);
	std::wstring cfname(t.begin(), t.end()); 

	CodaSysConfig.configchoice = 100;

	for (int iconfig = 0; iconfig <= 9; iconfig++)
		if(cfname.compare(configs.config[iconfig].strName))
			CodaSysConfig.configchoice = iconfig;

	if(CodaSysConfig.configchoice < 0 || CodaSysConfig.configchoice > 9)
	{
		std::cerr << "Unrecognized or Invalid Hardware Configuration specified in config file!" << std::endl;
		return(4);
	}

	// get enabled devices for selected hardware config
	CODANET_HWCONFIG_DEVICEENABLE devices;
	CodaSysConfig.cl.getDeviceEnable(configs.config[CodaSysConfig.configchoice].dwAddressHandle, devices);

	// print device IDs
	char strDevice[][20] = {"", "cx1", "AMTIserial", "GS16AIO", "Video direct", "Video remote", "Kistler", "AMTIanalog", "DI-720", "NI6221","","","","",""};
	std::cerr << "Found " << devices.dwNumOptions << "Available Device types:" << std::endl;
	for(DWORD idevice = 0; idevice < devices.dwNumOptions; idevice++)
	{
		WORD deviceID = (DWORD)devices.option[idevice].wDevice;
		//std::cerr << "   ID[" << idevice << "]: " << deviceID << strDevice[deviceID] << " (" << (devices.option[idevice].wEnable ? "enabled" : "disabled") << ")" << std::endl;
		fprintf(stderr, "  ID[%u]: %04X %10s [%s]\n", idevice, deviceID, strDevice[deviceID], devices.option[idevice].wEnable ? "enabled" : "disabled");
		if(deviceID == DEVICEID_GS16AIO && devices.option[idevice].wEnable)
			CodaSysConfig.bGS16AIOenabled = TRUE;
		if(deviceID == DEVICEID_NI6221 && devices.option[idevice].wEnable)
			CodaSysConfig.bNI6221enabled = TRUE;
	}

	// ensure that system is stopped (in case a client failed and left it open):
	CodaSysConfig.cl.stopSystem();

	// time system startup:
	clock_t t0, t1;
	t0 = clock();
	// if system is already started, this does nothing
	// otherwise it will load the first configuration
	CodaSysConfig.cl.startSystem(configs.config[CodaSysConfig.configchoice].dwAddressHandle);
	
	t1 = clock();
	std::cerr << "System started (" << (float)(t1 - t0)/CLOCKS_PER_SEC << ")" << std::endl;



	//*** marker alignment ***

	char doalign;
	DWORD alignmarkers[3];

	//read whether we are going to align the markers, and if so, what those markers are
	cfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the configuration
	sscanf(tmpline,"%c %u %u %u", &doalign, &alignmarkers[0], &alignmarkers[1], &alignmarkers[2]);

	if (doalign == 'y' || doalign == 'Y')
	{
		// alignment request structure
		codaRTNet::DeviceOptionsAlignment align(alignmarkers[0], alignmarkers[0], alignmarkers[1], alignmarkers[0], alignmarkers[2]);

		// perform alignment
		// this will cause system to look for alignment markers
		// - they must be in position by this time
		CodaSysConfig.cl.setDeviceOptions(align);

		// retrieve information
		codaRTNet::DeviceInfoAlignment info;
		CodaSysConfig.cl.getDeviceInfo(info);

		// print alignment diagnostics
		DWORD marker_id_array[5] = { alignmarkers[0], alignmarkers[0], alignmarkers[1], alignmarkers[0], alignmarkers[2] };

		print_alignment_status(marker_id_array, info);
		//fprintf(stdout, "\n");

		//TEMP:
		// Note that this is the number of sections in the alignment info loaded at startup, not the number of cx1s in use
		// (If there is no alignment data, this is zero.)
		std::cerr << "info.dev.NumUnits: " << info.dev.dwNumUnits << std::endl;

		// exit early if alignment failed
		if (info.dev.dwStatus != 0)
		{
			// shutdown system before exiting:
			std::cerr << "Alignment failed. Shutting down system..." << std::endl;
			CodaSysConfig.cl.stopSystem();
			return(5);
		}

	}

	//TEST: retrieve information:
	//Info from alignment data, if any: may not correspond to configured system
	codaRTNet::DeviceInfoAlignment info;
	CodaSysConfig.cl.getDeviceInfo(info);
	std::cerr << "DeviceInfoAlignment.dev.NumUnits: " << info.dev.dwNumUnits << std::endl;
	//Info about configured system:
	codaRTNet::DeviceInfoUnitCoordSystem devinfo;
	CodaSysConfig.cl.getDeviceInfo(devinfo);
	std::cerr << "DeviceInfoUnitCoordSystem.dev.NumUnits: " << devinfo.dev.dwNumUnits << std::endl;



	//*** Set up data stream ***

	// monitor-loop period (ms):
	// this determines the rate of monitor lines printed to the console during acquisition, and written to <monitor.txt>
    CodaSysConfig.MonitorPeriod = RUNSAMPRATE; //ms

    // create data stream (if not attempting a restart)
    // New to V1.7: New RTNet SDK function has new parameter to increase UDP buffer size, 
    // so can download more samples (packets) from acquisition buffers with one request.
	CodaSysConfig.cl.createDataStreamBuffer(CodaSysConfig.stream, 7000, CodaSysConfig.UDPbufferSize); //createDataStreamBuffer(DataStream& stream, ::WORD port, int bufferSize)



	//*** Specify whether to use External sync ***
	char dosync;

	cfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the configuration
	sscanf(tmpline,"%c", &dosync);

    CodaSysConfig.bExtSync = 0;
	if(dosync == 'Y' || dosync == 'y')
	{
      CodaSysConfig.bExtSync = TRUE;
	}


	//*** Set sampling rate, Marker IDs, and related parameters ***

	char samprate;
	CodaSysConfig.cx1mode = CODANET_CODA_MODE_100;
    CodaSysConfig.cx1decim = 1;

	cfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the configuration
	sscanf(tmpline,"%s %u", &samprate, &CodaSysConfig.cx1decim); //mode is the sampling rate, i.e. one of: (100, 200, 400, 800) Hz
	
	// choose mode and determine maximum number of markers
    switch(samprate)
    {
      default:
      case '100':
        CodaSysConfig.cx1mode = CODANET_CODA_MODE_100;
        CodaSysConfig.MaxMarkers = 56;
        break;
      case '200':
        CodaSysConfig.cx1mode = CODANET_CODA_MODE_200;
        CodaSysConfig.MaxMarkers = 28;
        break;
      case '400':
        CodaSysConfig.cx1mode = CODANET_CODA_MODE_400;
        CodaSysConfig.MaxMarkers = 12;
        break;
      case '800':
        CodaSysConfig.cx1mode = CODANET_CODA_MODE_800;
        CodaSysConfig.MaxMarkers = 6;
        break;
    }

	// request max Marker ID in use (limits datafile output; does not affect cx1 mode)
    CodaSysConfig.MaxMarkerInUse = CodaSysConfig.MaxMarkers-1;
    
	cfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the configuration
	sscanf(tmpline,"%u", &CodaSysConfig.MaxMarkerInUse); //mode is the sampling rate, i.e. one of: (100, 200, 400, 800) Hz

	std::cerr << "There are " << CodaSysConfig.MaxMarkerInUse << " markers requested for use." << std::endl;

	if(CodaSysConfig.MaxMarkerInUse <= 0)
		CodaSysConfig.MaxMarkerInUse = 1;
	else if(CodaSysConfig.MaxMarkerInUse > CodaSysConfig.MaxMarkers)
	{
		std::cerr << "  There are too many markers specified for requested sampling rate! "; 
		if (CodaSysConfig.MaxMarkerInUse > 56) //we are in any mode and more than 56 markers are in use; need to limit the total number of markers.
		{
			std::cerr <<  "Number of markers limited to 56. " << CodaSysConfig.MaxMarkerInUse - 56 << " markers not recorded." << std::endl;
			CodaSysConfig.MaxMarkerInUse = 56;
			CodaSysConfig.cx1mode = CODANET_CODA_MODE_100;
		}
		else if (CodaSysConfig.MaxMarkerInUse > 28) //we are in a mode greater than or equal to 200 and there are more than 28 markers in use; set to 100 Hz
		{
			std::cerr <<  "Sampling rate reduced to 100 Hz." << std::endl;
			CodaSysConfig.cx1mode = CODANET_CODA_MODE_100;
			CodaSysConfig.MaxMarkers = 56;
		}
		else if (CodaSysConfig.MaxMarkerInUse > 12) //we are in a mode greater than or equal to 400 and there are more than 12 markers in use; set to 200 Hz
		{
			std::cerr <<  "Sampling rate reduced to 200 Hz." << std::endl;
			CodaSysConfig.cx1mode = CODANET_CODA_MODE_200;
			CodaSysConfig.MaxMarkers = 28;
		}
		else if (CodaSysConfig.MaxMarkerInUse > 6) //we are in mode 800 and there are more than 6 markers in use; set to 400 Hz
		{
			std::cerr <<  "Sampling rate reduced to 400 Hz." << std::endl;
			CodaSysConfig.cx1mode = CODANET_CODA_MODE_400;
			CodaSysConfig.MaxMarkers = 12;
		}
	}


    if(CodaSysConfig.bExtSync)
       std::cerr << " External synching selected. Be sure that External sampling frequency does not exceed " << cx1mode << " Hz." << std::endl;
	else
      if(CodaSysConfig.cx1decim < 1 || CodaSysConfig.cx1decim > 9)
         CodaSysConfig.cx1decim = 1;

    // calculate & show acquisition frame-rate
    CodaSysConfig.AcqRate = (float)CodaSysConfig.cx1mode / (float)CodaSysConfig.cx1decim;
	/*
	std::cerr << "Mode: " << CodaSysConfig.cx1mode << ", decimation: " << CodaSysConfig.cx1decim << std::endl 
			  << "  Markers: " << CodaSysConfig.MaxMarkerInUse << "out of " << CodaSysConfig.MaxMarkers << " in use." << std::endl
			  << "  Acquisition rate: " << CodaSysConfig.AcqRate << " Hz." << std::endl;
	*/
	fprintf(stderr, "Mode:%d Decimation:%d AcqRate:%0.1fHz\n", CodaSysConfig.cx1mode, CodaSysConfig.cx1decim, CodaSysConfig.AcqRate);

    //-- set CodaMode (internal sync)
    codaRTNet::DeviceOptionsCodaMode codaMode1(CodaSysConfig.cx1mode, CodaSysConfig.cx1decim, CodaSysConfig.bExtSync);  // dwRateMode, dwDecimation, dwExternalSync
    CodaSysConfig.cl.setDeviceOptions(codaMode1);

    // get the sample tick period for cx1 data
    // (should be same as AcqRate)
    CodaSysConfig.cx1TickTime = CodaSysConfig.cl.getDeviceTickSeconds(DEVICEID_CX1);

    
    // request acquisition time limit:
    CodaSysConfig.MaxSamples = CODANET_ACQ_UNLIMITED;
    CodaSysConfig.AcqTimeMax = 0.0F;
	
	cfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the configuration
	sscanf(tmpline,"%f", &CodaSysConfig.AcqTimeMax); //mode is the sampling rate, i.e. one of: (100, 200, 400, 800) Hz
	
    if(CodaSysConfig.AcqTimeMax > 0.0f)
      CodaSysConfig.MaxSamples = (int)(CodaSysConfig.AcqTimeMax * CodaSysConfig.AcqRate);


    // limit acquisition to maxTicks samples of CX1 data, or not:
    if(CodaSysConfig.MaxSamples == 0)
       CodaSysConfig.MaxSamples = CODANET_ACQ_UNLIMITED;
	CodaSysConfig.cl.setAcqMaxTicks(DEVICEID_CX1, CodaSysConfig.MaxSamples);
	
	std::cerr << "Max acquisition time (s): " << CodaSysConfig.AcqTimeMax << std::endl;



	//*** Set up a file to record monitored data ***

	fopen_s(&CodaSysConfig.monitorfile, "monitor.txt", "wt");
    //FILE* monitorfile = fopen("monitor.txt", "wt");
    if(CodaSysConfig.monitorfile)
    {
		fprintf(CodaSysConfig.monitorfile, "Codamotion RTNet Buffered Acquisition monitored data.\n");
		if(CodaSysConfig.bExtSync)
			fprintf(CodaSysConfig.monitorfile, "External Sync.\n");
		fprintf(CodaSysConfig.monitorfile, "Acquisition: %0.1fs at %u/%u = %0.1fHz\nMonitored at %dms\n", 
                            CodaSysConfig.AcqTimeMax, CodaSysConfig.cx1mode, CodaSysConfig.cx1decim, CodaSysConfig.AcqRate, CodaSysConfig.MonitorPeriod);
    }



	//*** Start Acquisition ***

    // prepare for acquisition (includes cx1 sensor offset; takes ~500ms)
    t0 = clock();
	CodaSysConfig.cl.prepareForAcq();
    t1 = clock();
    std::cerr << "cl.prepareForAcq(): "<< (float)(t1 - t0)/CLOCKS_PER_SEC << "s" << std::endl;

	// ensure socket is up before starting acquisition
	codanet_sleep(50);

	// start buffered acquisition
    t0 = clock();
    CodaSysConfig.cl.startAcqContinuousBuffered();
    t1 = clock();
	std::cerr << "cl.startAcqContBuffered(): " << (float)(t1 - t0)/CLOCKS_PER_SEC << "s" << std::endl;
	CodaSysConfig.timeStart = clock();
	CodaSysConfig.MonitorSample = 0;
    CodaSysConfig.SampleNum = 0;
    CodaSysConfig.PrevSampleNum = 0;
    CodaSysConfig.SampleTime = 0.0F;

	CodaSysConfig.GetSampleTimer = new Timer();
	CodaSysConfig.GetSampleTimer->SetAlarmTime(CodaSysConfig.MonitorPeriod,1);
	CodaSysConfig.GetSampleTimer->Reset();
    memset(CodaSysConfig.SyncStopSampleNum, 0, sizeof(DWORD));

	//close configuration file
	cfile.close();

	return(0);
}



int TrackCoda::GetUpdatedSample(TrackDATAFRAME DataCodaFrame[])
{

    // acquisition counters & timers:
    //CodaSysConfig.timeStart = clock();
    //CodaSysConfig.timeStop = 0;

    // list of sample numbers where ext.sync stopped:
    int iSyncStop = 0;

	if(CodaSysConfig.GetSampleTimer->CheckAlarm())
	{
		//if the timer has stopped, a new sample should be available; we can then proceed to access it
		CodaSysConfig.GetSampleTimer->Reset(); //reset the timer


		// request latest frame (packet) from all devices
		try
		{
			// Request RTServer to transmit a packet of data from each device
			CodaSysConfig.cl.monitorAcqBuffer();
		}
		catch(codaRTNet::DeviceStatusArray& array)
		{
			/*... ignore status information from codanet_cmd_acqbuffermonitor -
			errors will just cause codanet_packet_receive to time out... */
			// not necessarily...
			print_devicestatusarray_errors(array);
			if(getfirstdeverror(array) != CODANET_DS_STATEERROR_ILLEGALCHANGE)
			{
				//A major error has occurred in the data acquisition request

				// stop system:
				CodaSysConfig.cl.stopAcq();      // leaves system in state.2 "Started", not in state.3 "Prepared"
				CodaSysConfig.cl.stopSystem();

				// close datastream:
				CodaSysConfig.cl.closeDataStream(CodaSysConfig.stream);
				CodaSysConfig.cl.skt = NULL;

				//print error message and quit
				std::cerr << "CODA fatal error when attempting to retrieve data from server." << std::endl;
				return(0);
			}
		}

		//a packet has been successfully sent. access the requested packet (or wait for N us)
		DWORD receiveTimeout = 100;  //us - if no packets are readily available, wait this duration and then timeout - command blocks during this time so we have to keep it short!
		bool bTimeout = FALSE;
		codaRTNet::RTNetworkPacket packet;
		while(!bTimeout)  //keep reading packets until we've cleared out the buffer.
		{
			// receive packets from all devices, until all packets have been received or until timeout
			if(CodaSysConfig.stream.receivePacket(packet, receiveTimeout) == CODANET_STREAMTIMEOUT)
			{
				// timed out since no more packets are available -
				// this usually means that packets from all devices have been received/processed (with a few exceptions, see below)
				bTimeout = TRUE;
			}
			else
			{ 
				// check result
				if(packet.verifyCheckSum())
				{
					// decode objects
					codaRTNet::PacketDecode3DResultExt decode3D;	// 3D measurements (CX1)
					codaRTNet::PacketDecodeADC16 decodeADC;			// 16-bit ADC measurements (GS16AIO or NI6221)

					// decode & print CX1 3D measurement sresults
					if(decode3D.canDecodePacket(packet) && decode3D.decode(packet))
					{
						// increment monitor samplenum only for cx1 device:
						CodaSysConfig.MonitorSample++;
						// get the cx1 sample number:
						CodaSysConfig.SampleNum = decode3D.getTick();

						// if SampleNum hasn't changed when running with ext.sync, don't re-print visibility data:
						if(CodaSysConfig.bExtSync && (CodaSysConfig.MonitorSample > 1) && (CodaSysConfig.SampleNum == CodaSysConfig.PrevSampleNum))
						{
							// console output only:
							fprintf(stderr, "Waiting for Ext.Sync...\r");  //use \r to prevent scrolling this message 
							// add to list of sync-stop sample numbers, for indication in buffer download data:
							CodaSysConfig.SyncStopSampleNum[iSyncStop] = CodaSysConfig.SampleNum;
						}
						else
						{
							// increment iSyncStop counter if last SampleNum was recorded:
							if(CodaSysConfig.SyncStopSampleNum[iSyncStop] != 0 && iSyncStop < 999)
								iSyncStop++;

							// remember this (new) sample number:
							CodaSysConfig.PrevSampleNum = CodaSysConfig.SampleNum;

							CodaSysConfig.SampleTime = CodaSysConfig.SampleNum * CodaSysConfig.cx1TickTime;

							// find number of marker positions available
							DWORD NumMarkers = decode3D.getNumMarkers();
							//BYTE  valid[56]; //max possible Markers is 56
							char  strInView[65] = "123456789 0123456789 0123456789 0123456789 0123456789 0123456";
							int i = 0;

							// monitor-data datafile (if any):
							if(CodaSysConfig.monitorfile)
								fprintf(CodaSysConfig.monitorfile, "%04d: %04d %08.5fs Visibility[%d]: ", CodaSysConfig.MonitorSample, CodaSysConfig.SampleNum, CodaSysConfig.SampleTime, NumMarkers);

							// console output monitor:
							//fprintf(stderr, "%04d: %04d %09.5fs Visibility[%d]: ", CodaSysConfig.MonitorSample, CodaSysConfig.SampleNum, CodaSysConfig.SampleTime, NumMarkers);

							// loop through marker data:
							for(DWORD imarker = 0; imarker < NumMarkers; imarker++)
							{
								DataCodaFrame[imarker].ValidInput = (decode3D.getValid(imarker))? 1:0;
								DataCodaFrame[imarker].time = CodaSysConfig.SampleNum;
								
								//calculate average 
								DataCodaFrame[imarker].CodaNumCameras = decode3D.getNumCamerasPerMarker();
								BYTE *intensity = decode3D.getIntensity(imarker);
								for(int icamera = 0; icamera < DataCodaFrame[imarker].CodaNumCameras; icamera++)
									DataCodaFrame[imarker].CodaIntensity = DataCodaFrame[imarker].CodaIntensity + intensity[icamera];
								DataCodaFrame[imarker].CodaIntensity = DataCodaFrame[imarker].CodaIntensity/DataCodaFrame[imarker].CodaNumCameras;

								// edit in-view summary string for console output and monitorfile:
								// (summary string has a gap every 10th marker)
								i = imarker + (imarker+1)/10;
								if(!DataCodaFrame[imarker].ValidInput)
									strInView[i] = '-';

								// save marker data into data structure
								float* pos = decode3D.getPosition(imarker);
								DataCodaFrame[imarker].x = pos[0];
								DataCodaFrame[imarker].y = pos[1];
								DataCodaFrame[imarker].z = pos[2];

							}
							// terminate strInView:
							strInView[i+1] = 0;

							// console output monitor:
							fprintf(stderr, "%s\n", strInView);

							// monitor datafile:
							if(CodaSysConfig.monitorfile)
								fprintf(CodaSysConfig.monitorfile, "%s\n", strInView);
						}
					}
				}
				else
				{
					fprintf(stderr, "checksum failed\n");
				}
			}
		} //all available packets in the stream have been cleared; this prevents packet buildup and ensures we have the most recent packet to work with at all times

		if(bTimeout)
		{
			// cx1 may have stopped acquiring if cl.setAcqMaxTicks() has been set < CODANET_ACQ_UNLIMITED
			// (cl.isAcqInProgress() is slow, so don't call frequently, only when receivePacket() has timed out)
			if(!CodaSysConfig.cl.isAcqInProgress())
			{
				CodaSysConfig.timeStop = clock();
				return(-1); //we have exceeded the maximum recording time; data is no longer available!
			}
			// if cx1 is still acquiring in ext. sync mode, then timeout is because no sync has been received:
			//!!What if there is another device, such as ADC?
			if(CodaSysConfig.bExtSync)
			{
				std::cerr << "CODA Waiting for External Sync..." << std::endl;
			}
		}

		return(1); //most recent data sample has been successfully received and parsed
	}

	else
		//a new sample is not yet available; return null
		return(0);


}

bool TrackCoda::ShutDownCoda()
{

	// stop acquisition if still in progress
	if(CodaSysConfig.cl.isAcqInProgress())
	{
		CodaSysConfig.timeStop = clock();
		CodaSysConfig.cl.stopAcq();       // leaves system in state.2 "Started", not in state.3 "Prepared"
	}


    float ElapsedTime = (float)(CodaSysConfig.timeStop - CodaSysConfig.timeStart)/CLOCKS_PER_SEC;
    std::cerr << "End of CODA acquisition: " << ElapsedTime << "s." << std::endl;
    
	// close the monitor-data datafile:
    if(CodaSysConfig.monitorfile)
    {
      fprintf(CodaSysConfig.monitorfile, "\nElapsed time: %0.3fs\n", ElapsedTime);  
      fclose(CodaSysConfig.monitorfile);
    }

    // Download data buffers: -------------------------------------------------------------------
    DWORD NumSamplesCX1 = CodaSysConfig.cl.getAcqBufferNumPackets(DEVICEID_CX1);
    DWORD NumSamplesADC = 0;
    if(CodaSysConfig.bGS16AIOenabled)
		NumSamplesADC = CodaSysConfig.cl.getAcqBufferNumPackets(DEVICEID_GS16AIO);

    std::cerr << "Downloading " << NumSamplesCX1 << " samples from acq.buffer: ";

    // create datafile name:
    char datafile[200] = "";
	
	std::string savfile;
	savfile.assign(TRIALFILE);
	savfile.replace(savfile.rfind("."),4,"_data");
	//std::strcpy(datafile,savfile.c_str());

	time_t current_time = time(0);
	tm* ltm = localtime(&current_time);
	std::stringstream ss1, ss2, ss3;
	
	ss1 << std::setw(4) << std::setfill('0') << ltm->tm_year + 1900;
	ss1 << std::setw(2) << std::setfill('0') << ltm->tm_mon + 1;
	ss1 << std::setw(2) << std::setfill('0') << ltm->tm_mday;
	
	ss2 << std::setw(2) << std::setfill('0') << ltm->tm_hour;
	ss2 << std::setw(2) << std::setfill('0') << ltm->tm_min;
	ss2 << std::setw(2) << std::setfill('0') << ltm->tm_sec;

	ss3 << savfile.c_str() << "_" << ss1.str() << ss2.str() << ".dat";

    // open output datafile:
    //FILE* fp = 0; 
	//fopen_s(&fp, datafile, "wt");
    FILE* fp = fopen(ss3.str().c_str(), "wt");
	if(fp)
	{
		fprintf(fp, "Codamotion Marker data\n");
		fprintf(fp, "NumMarkers:\t%d\n", CodaSysConfig.MaxMarkerInUse);
		fprintf(fp, "NumSamples:\t%d\n", NumSamplesCX1);
		fprintf(fp, "AcqMode (Hz):\t%u\n", CodaSysConfig.cx1mode);
		fprintf(fp, "AcqTime (s):\t%0.3f\n", CodaSysConfig.AcqTimeMax);
		if(CodaSysConfig.bExtSync)
		  fprintf(fp, "External Sync:\t1\n");
		else
		  fprintf(fp, "AcqRate (Hz):\t%0.3f\n", CodaSysConfig.AcqRate);
		fprintf(fp, "NumSamplesADC:\t%d\n", NumSamplesADC);

		// column headers:
		fprintf(fp, "Sample\tTime");
		for(int m = 0; m < CodaSysConfig.MaxMarkerInUse; m++)
		  fprintf(fp, "\tM%d\tX\tY\tZ", m+1);  //Marker ID is 1-based in datafile
		fprintf(fp, "\n");
	}
    DWORD iStop = 0;

    // time the cx1 data download:
    CodaSysConfig.timeStart = clock();

    //-- request all samples:
    //-- CX1 packets have DataSize=452 and trasmitsize=468 bytes if not acquiring multi-coda data
    // add 1 for safety, if UDP buffer is N * n
    DWORD NumSamplesPerRequest = CodaSysConfig.UDPbufferSize / 469;  //!!Need to change this if multi-coda data is acquired: packets will be larger
    DWORD sample = 0;

    while(sample < NumSamplesCX1)
    {
      //-- request a bunch of samples:
	  // clrequestAcqBufferPacketAll(WORD device, DWORD startSample, DWORD endsample) 
      DWORD lastsample = sample + NumSamplesPerRequest;
      if(lastsample >= NumSamplesCX1)
        lastsample = NumSamplesCX1 - 1;
      CodaSysConfig.cl.requestAcqBufferPacketAll(DEVICEID_CX1, sample, lastsample);
//TEMP:
      fprintf(fp, "request(%d,%d)\n", sample, lastsample);  

      //-- download the bunch of samples: 
      while(sample <= lastsample)
      {
		  codaRTNet::RTNetworkPacket packet;
      DWORD receiveTimeout = 1000; //us
	    if(CodaSysConfig.stream.receivePacket(packet, receiveTimeout) == CODANET_STREAMTIMEOUT)
         fprintf(fp, "\ntimeout:%u ", receiveTimeout);

//#if !NOWRITEDATA
        else
        {
// check packetsize:
//          DWORD psize = packet.getDataSize();
//          DWORD tsize = packet.transmitSize();
//          fprintf(fp, "packet.DataSize: %u  packet.transmitSize: %u \n", psize, tsize);

			    // check result
			    if(packet.verifyCheckSum())
			    {
				    // decode objects
				    codaRTNet::PacketDecode3DResultExt decode3D;	// 3D measurements (CX1)
				    // decode & print/store results
				    if(decode3D.decode(packet))
				    {
						// get the sample number:
						CodaSysConfig.SampleNum = decode3D.getTick();
						CodaSysConfig.SampleTime = CodaSysConfig.SampleNum * CodaSysConfig.cx1TickTime;

						// insert a blank line if external sync paused at this sampleNum:
						if(CodaSysConfig.bExtSync && (iStop < 1000) && (CodaSysConfig.SyncStopSampleNum[iStop] == CodaSysConfig.SampleNum))
						{
							fprintf(fp, "\n");
							iStop++;
						}
              
						// write data to file:
						fprintf(fp, "%u\t%0.5f", (CodaSysConfig.SampleNum + 1), CodaSysConfig.SampleTime);

						// get number of marker data in packet:
					    DWORD NumMarkers = decode3D.getNumMarkers();
						DWORD MaxMarker = (NumMarkers < CodaSysConfig.MaxMarkerInUse)? NumMarkers : CodaSysConfig.MaxMarkerInUse;

						BYTE valid[56];  //max possible Markers is 56

					    for(DWORD marker = 0; marker < MaxMarker; marker++)
					    {
						    float* pos = decode3D.getPosition(marker);
							valid[marker] = (decode3D.getValid(marker))? 1:0;
						    BYTE* intensity = decode3D.getIntensity(marker);
							// write data to file:
							fprintf(fp, "\t%u\t%0.1f\t%0.1f\t%0.1f", valid[marker], pos[0], pos[1], pos[2]);
					    }
						// datfile newline:
						fprintf(fp, "\n");
				    }
			    }
			    else
			    {
					fprintf(stderr, "\r%d: checksum failed.", sample);
			    }
			}
//#endif
			//-- increment sample index:
			sample++;
		}
    }
    CodaSysConfig.timeStop = clock();
    ElapsedTime = (float)(CodaSysConfig.timeStop - CodaSysConfig.timeStart)/CLOCKS_PER_SEC;
    fprintf(stderr, "Total CODA File Download Time: %0.3fs (%0.1f samples/s)\n", ElapsedTime, (float)NumSamplesCX1/ElapsedTime);  
    //fprintf(fp, "\nDownload time: %0.3fs (%0.1f samples/s)\n", ElapsedTime, (float)NumSamplesCX1/ElapsedTime);  

    // close datafile:
    fclose(fp);


	
	// shutdown system before exiting:
	std::cerr << "Shutting down CODA system..." << std::endl;
	CodaSysConfig.cl.stopSystem();


}




void TrackCoda::print_alignment_status(const DWORD* marker_id_array,  const codaRTNet::DeviceInfoAlignment& info)
{
		// print alignment status value
		std::cerr << "Alignment result: ";
		switch (info.dev.dwStatus)
		{
		case 0:
			std::cerr << "success!";
			break;
		case CODANET_ALIGNMENTERROR_SYSTEM:
			std::cerr << "system error!";
			break;
		case CODANET_ALIGNMENTERROR_ALREADY_ACQUIRING:
			std::cerr << "already acquiring (is another program running?";
			break;
		case CODANET_ALIGNMENTERROR_OCCLUSIONS:
			std::cerr << "occlusions.");
			break;
		case CODANET_ALIGNMENTERROR_XTOOCLOSE:
			std::cerr << "x-axis markers too close.";
			break;
		case CODANET_ALIGNMENTERROR_XYTOOCLOSE:
			std::cerr << "xy-plane markers too close";
			break;
		case CODANET_ALIGNMENTERROR_NOTPERP:
			std::cerr << "marked axes not sufficiently perpendicular";
			break;
		default:
			std::cerr << "unknown alignment status error code " << info.dev.dwStatus;
			break;
		}
		std::cerr << std::endl;

		// number of CX1 units
		DWORD nunits = info.dev.dwNumUnits;

		// frame count
		DWORD nframes = info.dev.dwNumFrames;

		// print visibility information
		for (DWORD icoda = 0; icoda < nunits; icoda++)
		{
			// index of Codamotion CX1 unit
			std::cerr << "Coda " << icoda+1 << std::endl;

			// data from each marker
			for (DWORD imarker = 0; imarker < 5; imarker++)
			{
				// actual marker identity
				DWORD marker_identity = marker_id_array[imarker];

				for (DWORD icam = 0; icam < 3; icam++)
				{
					// label for this marker
					switch (imarker)
					{
					case 0:
						std::cerr << "  Org ";
						break;
					case 1:
						std::cerr << "  X0 ";
						break;
					case 2:
						std::cerr << "  X1 ";
						break;
					case 3:
						std::cerr << "  XY0 ";
						break;
					case 4:
						std::cerr << "  XY1 ";
						break;
					}

					// print marker identity
					std::cerr << "(Marker " << marker_identity << ") ";

					// camera letter for each coda camera
					switch (icam)
					{
					case 0:
						std::cerr << "A: ";
						break;
					case 1:
						std::cerr << "B: ";
						break;
					case 2:
						std::cerr << "C: ";
						break;
					}

					// print visibility graph for frames of data
					// show a 1 for visible and _ for occluded
					// (show alternate frames only to save screen space)
					for (DWORD iframe = 0; iframe < nframes; iframe+=2)
					{
						BYTE flag = info.dev.camera_flag[3*nframes*5*icoda + 3*nframes*imarker + 3*iframe + icam];
						if (flag <= 10)
							std::cerr << "_";
						else
							std::cerr << "1";
					}

					// new line
					std::cerr << std::endl;
				}
			}
		}
}


void TrackCoda::print_devicestatusarray_errors(const codaRTNet::DeviceStatusArray& array)
{
	for (DWORD idev = 0; idev < array.numstatus; idev++)
	{
		if (array.status[idev].error)
		{
			fprintf(stderr, "DEVICE %u SUBSYSTEM %u ERROR: %u\n", 
				(DWORD)array.status[idev].deviceID, 
				(DWORD)array.status[idev].subsystemID,
				(DWORD)array.status[idev].error);
		}
	}
}

DWORD TrackCoda::getfirstdeverror(const codaRTNet::DeviceStatusArray& arr)
{
  DWORD error = 0;
	for (DWORD idev = 0; idev < arr.numstatus; idev++)
	{
		if(arr.status[idev].error)
		  error = (DWORD)arr.status[idev].error;
	}
  return error;
}