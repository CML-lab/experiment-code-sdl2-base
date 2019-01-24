#ifndef VLCVIDEOPLAYER_H
#define VLCVIDEOPLAYER_H
#pragma once

#include <SDL.h>
#include <SDL_mutex.h>
#include "SDL_filesystem.h"
#include "config.h"
#include <iostream>
#include <sstream>
#include "vlc/vlc.h"


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
	
	//some VLC parameters
	libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
	libvlc_event_manager_t *mpevent;
	libvlc_state_t mpstate;

	//some status parameters
	int hasStarted;
	int hasEnded;

public:

	Video(const char* fname, int x, int y, int w, int h);  //constructor function
	//int Init();
	~Video() { };
	
	void SetPos(int x, int y); //function to set the position of the video on the screen

	int GetStatus();	//function to get the current status (updates and returns VLC state)
	int HasStarted();	//function to find out if video playing has started
	int HasEnded();		//function to find out if video playing has ended or has been stopped
	void ResetStatus();	//reset the status flags - does NOT affect the actual video status

	int Play();		//start playing the video
	int Stop();		//stop playing the video
	int Pause();	//pause playing the video
	int ResetVid();	//Reset the video back to the beginning for the next playback
	void Visible();	//make the video window visible
	void Invisible(); //make the video window invisible

	void CleanUp();	//cleans up VLC stuff

};

#endif
