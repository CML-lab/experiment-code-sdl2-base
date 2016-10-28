#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#include "SDL_opengl.h"

// Configurable constants

//screen dimensions
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT  1080
//#define SCREEN_WIDTH  1280
//#define SCREEN_HEIGHT  720
#define SCREEN_BPP  32
#define WINDOWED  false
#define MIRRORED  true
// Physical dimensions of the screen in meters
#define PHYSICAL_WIDTH  1.21f  //123.825
#define PHYSICAL_HEIGHT  0.68f // meters  //68.58
// screen ratio, in meters per pixel
#define PHYSICAL_RATIO  (PHYSICAL_WIDTH / SCREEN_WIDTH)


//target dimensions, meters - training blocks
#define CURSOR_RADIUS  0.0045f

//TRACKTYPE: type of tracker being used: FOB (0) or trakStar (1)
#define TRACKTYPE 0
#define BIRDCOUNT 4
#define SAMPRATE 130
#define FILTER_WIDE true 
#define FILTER_NARROW false
#define FILTER_DC 1.0f
//default filters: WIDE = true, NARROW = false; DC = 1.0f


//**define the trace-file path**
#define TRACEPATH "Resources/calibration"


//system constants
#define GROUP_ID  1
#define READ_TIMEOUT  2000
#define WRITE_TIMEOUT  2000
#define BAUD_RATE  115200
#define BIRDCOUNT  4

#define CALIBTIME 3600000

#define SAMPRATE 130

//calibration parameters -- consult AW for calibration routine

#define CALxOFFSET 0.0f   //meters
#define CALyOFFSET 0.0f   //meters
#define CALxyROTANG 0.0f //radians



#endif
