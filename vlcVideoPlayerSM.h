#ifndef VLCVIDEOPLAYERSM_H
#define VLCVIDEOPLAYERSM_H
#pragma once

#include <SDL.h>
#include <SDL_mutex.h>
#include "SDL_filesystem.h"
#include "config.h"
#include <iostream>
#include <sstream>
#include "vlc/vlc.h"
#include <Shlwapi.h>
#include <direct.h>

//be sure to add Shlwapi.lib in the linker properties!

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



class VideoSM
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

	VideoSM(const char* fname, int x, int y, int w, int h, int* errorcode);  //constructor function
	//int Init();
	~VideoSM() { };
	
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

	void CleanUp();	//cleans up VLC stuff

};

#endif
