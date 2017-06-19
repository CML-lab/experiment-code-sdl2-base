#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#include "SDL.h"
#include "SDL_opengl.h"
#include <vector>

////////// Set these parameters only! //////////////////////////

//set this to the data path in which the current set of data for this block should be stored.

//set this parameter to determine the active sensor to record from. 3 is the right hand, 1 is the left hand. 4 is for the LED-tracker. 0 is the mouse.
#define HAND 3

//define the file that contains the name of the trial table
#define TRIALFILE "test.txt"  


//define trace paths
#define TRACEPATH "Resources/traces"
#define NTRACES 1

#define PATHPATH "Resources/paths"
#define NPATHS 8

#define REGIONPATH "Resources/regions"
#define NREGIONS 1

// */



////////////////////////////////////////////////////////////////




// Configurable constants

//TRACKTYPE: type of tracker being used: FOB (0) or trakStar (1)
#define TRACKTYPE 0
#define BIRDCOUNT 4
//SAMPRATE: sampling rate (for 1 full measurement cycle, or time to activate all 3 axes.  for trakSTAR, we get samples returned at SAMPRATE*3 frequency.
#define SAMPRATE 130
//1 out of every REPORTRATE samples will be acquired from the trakSTAR, when in Synchronous mode
#define REPORTRATE 1
#define FILTER_WIDE false
#define FILTER_NARROW false
#define FILTER_DC 0.0f


//screen dimensions
//#define SCREEN_WIDTH  1920
//#define SCREEN_HEIGHT  1080
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
#define NSCREEN 3
typedef struct window_data
{
	int displayIndex;
    SDL_Rect bounds;
    SDL_Window *window;
	SDL_GLContext glcontext;
} SCREEN_struct;
extern SCREEN_struct screens[NSCREEN];

#if TRACKTYPE == 0

	// Physical dimensions of the screen in meters
	#define PHYSICAL_WIDTH  1.21f
	#define PHYSICAL_HEIGHT  0.68f // meters

#else

	// Physical dimensions of the screen in meters
	#define PHYSICAL_WIDTH  0.930f
	#define PHYSICAL_HEIGHT  0.523f // meters

#endif

#define SCREEN_BPP  32
//switch WINDOWED and MIRRORED for kinereach/simulation runs
#define WINDOWED  false
#define MIRRORED  true
// screen ratio, in meters per pixel
#define PHYSICAL_RATIO  (PHYSICAL_WIDTH / SCREEN_WIDTH)
//#define PHYSICAL_RATIOI  (SCREEN_WIDTH/PHYSICAL_WIDTH)


 //target dimensions, meters - practice blocks
#define START_RADIUS  0.006f
#define TARGET_RADIUS  0.006f
#define CURSOR_RADIUS  0.003f


//wait times
#define WAITTIME 1200
#define FIXTIME 1800 
#define DISPTIME 800
#define INTROTIME 8000
//#define LATTIME 1200

//define pi as a float
#define PI 3.14159265358979f

//calibration parameters -- consult AW for calibration routine
#if TRACKTYPE == 0

	//Flock of Birds calibration performed 4/12/2016 by Aaron Wong using pumbline method and modified calibration routine.
	#define CALxOFFSET 0.61350f   //meters
	#define CALyOFFSET 0.17690f   //meters
	#define CALxyROTANG -0.01938f //radians

#else

	//TrakStar calibration 
	#define CALxOFFSET 0.45945f   //meters
	#define CALyOFFSET 0.57198f   //meters
	#define CALxyROTANG -0.01031f  //radians

#endif

//velocity threshold values
#define VELMIN 0.4f 
#define VELMAX 1.3f
#define VELBARMIN 0.0f
#define VELBARMAX 2.0f
//define the velocity threshold for detecting movement onset/offset
#define VEL_MVT_TH 0.06f
//define the time that vel must be below thresh to define movement end, in msec
#define VEL_END_TIME 40

#define MAX_TRIAL_DURATION 1500
#define HOLDTIME 800

#endif
