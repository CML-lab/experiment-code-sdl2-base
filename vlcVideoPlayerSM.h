#ifndef VLCVIDEOPLAYERSM_H
#define VLCVIDEOPLAYERSM_H
#pragma once

/************************************************************************
This version of the VLC Video Player uses a state machine, rather than
 relying on asynchronous calls to the libvlc. The reason for this change
 is to improve the stability of the object and reduce deadlocking arising
 from too many successive calls to libvlc media player that can cause race
 conditions. Here, we wait for acknowledgements about changes in state before
 continuing with various multi-step operations such as resetting the video. 

 This approach appears to work, but has not been fully tested as of 3/6/2019.
 To implement this object, use the function calls to request changes in state
 of the video player (e.g., play, stop, reset, etc) but also remember to call
 Update() on every loop (e.g., in conjunction with each pass through the main
 state machine controlling the experiment) to keep the Video Player state
 machine updated and to allow all commands to complete for a given state change
 request. 

 Note, this player does NOT require VLC to be installed on the current computer.
 However, it may require that a copy of the VLC plugins folder be placed in the
 same parent folder as the program executable (along with libvlc.dll and 
 libvlccore.dll). It also requires that Shlwapi.lib be added to the linker properties.


 */



#include <SDL.h>
#include <SDL_mutex.h>
#include "SDL_filesystem.h"
#include "config.h"
#include <iostream>
#include <sstream>
#include "vlc/vlc.h"
#include <Shlwapi.h>



struct ctx {
	SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_mutex *mutex;
	SDL_Rect rect;
	float xpos;
	float ypos;
	int showVideo;
};


//to draw other stuff in the video
static void (*drawfunc)(SDL_Renderer *renderer);  //function pointer to a function that draws other stuff
void DrawOtherStuff(void (*func)(SDL_Renderer *renderer)); //function call that allows us to store the draw-function pointer

//callback functions
static void* lock(void *data, void **p_pixels);
static void unlock(void *data, void *id, void *const *p_pixels);
static void display(void *data, void *id);



class Video
{
private:
	struct ctx context; //Video/SDL context
	
	//State machine states
	enum VidState
	{
		Load = 0x01,		//00001
		Idle = 0x02,		//00010
		Playing = 0x04,		//00100
		Paused = 0x05,		//00101
		Stopped = 0x06,		//00110
		Ended = 0x07,		//00111
		Rewind = 0x12		//01100
	};


	VidState state;
	bool enteredstate;
	int requestplay;
	int requeststop;
	int requestpause;
	int requestrewind;
	int requestload;

	//some VLC parameters
	libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
	libvlc_event_manager_t *mpevent;
	libvlc_state_t mpstate;

	//some status parameters
	int hasStarted;
	int hasEnded;
	int hasStopped;

	int VidLoad(const char* fname);
	
	int isValid;  //flag to identify that the video is actually valid!
	int isVisible;
	Uint32 visTime;
	float vidPos;

public:

	Video(const char* fname, int x, int y, int w, int h, int* errorcode);  //constructor function
	//int Init();
	~Video() { };
	
	void SetPos(int x, int y); //function to set the position of the video on the screen

	int GetStatus();	//function to get the current status (updates and returns VLC state)
	int IsValid();		//querry if the current video is valids
	int HasStarted();	//function to find out if video playing has started
	int HasEnded();		//function to find out if video playing has ended
	int HasStopped();	//function to find out if the video playing has stopped
	void ResetStatus();	//reset the status flags - does NOT affect the actual video status
	void SetValidStatus(int status);

	int Play();		//request start playing the video
	int Stop();		//request stop playing the video
	int Pause();	//request pause playing the video
	int LoadNewVid(const char* fname);	//load a new video using the same context

	int Update();	//update the state machine
	int ResetVid();	//Reset the video back to the beginning for the next playback
	
	void Visible();	//make the video window visible
	void Invisible(); //make the video window invisible
	int VisibleState(); //querry the visibility state

	void CleanUp();	//cleans up VLC stuff

};

#endif
