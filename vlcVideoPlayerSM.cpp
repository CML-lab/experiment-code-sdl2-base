#include "vlcVideoPlayerSM.h"


void DrawOtherStuff(void (*func)(SDL_Renderer *renderer))
{
	 drawfunc = func;
}


// VLC prepares to render a video frame.
void* lock(void *data, void **p_pixels) {

    struct ctx *c = (ctx *)data;

    int pitch;
    SDL_LockMutex(c->mutex);
    SDL_LockTexture(c->texture, NULL, p_pixels, &pitch);

    return NULL; // Picture identifier, not needed here.
}


// VLC just rendered a video frame.
void unlock(void *data, void *id, void *const *p_pixels) {

    struct ctx *c = (ctx *)data;

    uint16_t *pixels = (uint16_t *)*p_pixels;

    // We can also render stuff.
	/*
    int x, y;
    for(y = 10; y < 40; y++) {
        for(x = 10; x < 40; x++) {
            if(x < 13 || y < 13 || x > 36 || y > 36) {
                pixels[y * VIDEOWIDTH + x] = 0xffff;
            } else {
                // RV16 = 5+6+5 pixels per color, BGR.
                pixels[y * VIDEOWIDTH + x] = 0x02ff;
            }
        }
    }
	*/

    SDL_UnlockTexture(c->texture);
    SDL_UnlockMutex(c->mutex);
}

// VLC wants to display a video frame.
void display(void *data, void *id) {

    struct ctx *c = (ctx *)data;

    //c->rect.w = VIDEOWIDTH;
    //c->rect.h = VIDEOHEIGHT;
	//c->rect.x = c->xpos - c->rect.w/2;
	//c->rect.y = c->ypos - c->rect.h/2;

	if (c->showVideo)
	{
		SDL_ShowWindow(c->window);
		SDL_SetRenderDrawColor(c->renderer, 255, 255, 255, 255);
		SDL_RenderClear(c->renderer);
		SDL_RenderCopy(c->renderer, c->texture, NULL, &(c->rect));
		
		if (drawfunc  != NULL)
			drawfunc(c->renderer);
		/*SDL_Rect rect;
		rect.w = SCREEN_WIDTH/2;
		rect.h = SCREEN_HEIGHT/12;
		rect.x = SCREEN_WIDTH/2 - rect.w/2;
		rect.y = SCREEN_WIDTH/2 - rect.h/2;
	
		SDL_SetRenderDrawColor(c->renderer, 255, 0, 0, 255);
		//SDL_RenderClear(c->renderer);
		SDL_RenderDrawRect(c->renderer, &rect);
		*/
	
		SDL_RenderPresent(c->renderer);
	}
	else
	{
		SDL_SetRenderDrawColor(c->renderer, 255, 255, 255, 255);
		SDL_RenderClear(c->renderer);
		SDL_HideWindow(c->window);
	}
}




Video::Video(const char* fname, int x, int y, int w, int h, int* errorcode) //SDL_Renderer *renderer, SDL_Texture *texture, SDL_mutex *mutex, SDL_Rect rect)
{
	//Note, this constructor does the "bad" thing of potentially failing, in which case it does not return a valid initialized object.
	//We need an external way to catch when this happens. To do so, we will pass it an "errorcode" that it will write into before it exits.
	//Then we can look at the value of the error code after calling the constructor. If the error code is 0, the constructor succeeded;
	//otherwise, the value of the error code indicates at what stage the initialization failed.

	*errorcode = 0;

	isValid = 0;


	//we need absolute paths, so we must figure out the project directory
	char *bpath = SDL_GetBasePath();
	std::string basepath;
	basepath.assign(bpath);
	//std::cerr << "BasePath: " << basepath.c_str() << std::endl;
	basepath.erase(basepath.rfind("\\"),1); //get rid of the last slash in the path
	basepath.erase(basepath.rfind("\\")+1,10); //get rid of the "Debug" folder name to get to the project folder
	//std::cerr << "ModBasePath: " << basepath.c_str() << std::endl;

    //Set the environmental variable to point to the VLC plugins directory
    //   If this isn't set properly, libvlc_new() will not work!
	//NOTE: Sometimes VLC still won't initialize even with this workaround; if libvlc_new() returns NULL,
	//      just drop a copy of the VLC plugins folder into the Debug folder.
	std::string envpathcmd;
	envpathcmd.assign("VLC_PLUGIN_PATH=");
	envpathcmd.append(basepath.c_str());
	envpathcmd.append("vlc-3.0.6\\plugins");
	_putenv(envpathcmd.c_str());
	std::cerr << "VLC_PLUGIN_PATH=" << getenv("VLC_PLUGIN_PATH") << std::endl;	
    //printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

	//we will set up a dedicated window and renderer for the video, as it plays asynchronously
	context.window = SDL_CreateWindow(
            "Vidplayer",
            float(x)-float(w)/2.0f,
            float(y)-float(h)/2.0f,
            w, h,
            SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SKIP_TASKBAR); // 
    if (!context.window)
	{
        std::cerr << "Couldn't create window: " << SDL_GetError() << std::endl;
		*errorcode = 1;
		isValid = 0;
		//return;
	}

    context.renderer = SDL_CreateRenderer(context.window, -1, 0);
    if (!context.renderer){
        std::cerr << "Couldn't create renderer: " << SDL_GetError() << std::endl;
		*errorcode = 2;
		isValid = 0;
		//return;
	}

	//set the window position in absolute pixel space on the screen
	context.xpos = x;
	context.ypos = y;
	
	//create a texture to put the video in, which is the same size as the window
	context.texture = SDL_CreateTexture(
            context.renderer,
            SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STREAMING,
            w, h);
    if (!context.texture) {
        std::cerr << "Couldn't create texture: " << SDL_GetError() << std::endl;
		*errorcode = 3;
		isValid = 0;
		//return;
    }

    context.mutex = SDL_CreateMutex();

	//set the video frame to fill the texture/window
	context.rect.w = w;
	context.rect.h = h;
	context.rect.x = 0;
	context.rect.y = 0;
	
	//set up some libVLC initialization parameters
	
	char const *vlc_argv[] = {
        "--no-audio", // Don't play audio.
        "--no-xlib", // Don't use Xlib.
        // Apply a video filter.
        //"--video-filter", "sepia",
        //"--sepia-intensity=200"
    };
    int vlc_argc;
	

	//initialize the special drawfunc to be null
	drawfunc = NULL;
	
	vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
	//std::cout << vlc_argc << " - " << vlc_argv[0] << " " << vlc_argv[1] << std::endl;
	libvlc = libvlc_new(vlc_argc, vlc_argv);
	
	// Initialise libVLC.
    //libvlc = libvlc_new(0, NULL);
    if(libvlc == NULL) {
		std::cerr << "LibVLC initialization failure." << std::endl;
		*errorcode = 4;
		isValid = 0;
		return;
		//exit;
        //return EXIT_FAILURE;
    }

	*errorcode = LoadNewVid(fname);
	//if we got this far, isValid has been set by the LoadNewVid function so we don't need to reset it
	//isValid = 1;

	std::cerr << "Video: " << fname << " load complete: status = " << *errorcode << "." << std::endl;

	//set up the State Machine and initialize SM flags
	state = Idle;
	enteredstate = false;
	
	requestplay = 0;
	requeststop = 0;
	requestpause = 0;
	requestrewind = 0;
	requestload = 0;
	
}


//function to load new video (only calls libvlc, not affect SDL window
int Video::LoadNewVid(const char* fname)
{
	int status = 0;

	requestload = 1;
	//status = VidLoad(fname);

	int w = context.rect.w;
	int h = context.rect.h;

	if (isValid > 0)
	{
		
		// Stop stream and clean up libVLC.
		if (hasStopped == 0)
			libvlc_media_player_stop(mp);

		libvlc_media_player_release(mp);
	}
	
	//set up the video file to be played
	//libVLC wants an absolute path, so we will figure out the project directory
	char *bpath = SDL_GetBasePath();
	std::string basepath;
	basepath.assign(bpath);
	//std::cerr << "BasePath: " << basepath.c_str() << std::endl;
	basepath.erase(basepath.rfind("\\"),1); //get rid of the last slash in the path
	basepath.erase(basepath.rfind("\\")+1,10); //get rid of the "Debug" folder name to get to the project folder
	//std::cerr << "ModBasePath: " << basepath.c_str() << std::endl;

	std::stringstream vidpath;
	int d = 0;
	vidpath << basepath.c_str() << VIDEOPATH << fname; //"\\Video" << d << ".divx";
	std::cerr << "VidPath: " << vidpath.str().c_str() << std::endl;

	//check if the path exists. For some reason libVLC doesn't do this check correctly so we have to do it manually!
	if (!PathFileExistsA(vidpath.str().c_str()))
	{
		std::cerr << "Video file/path does not exist." << std::endl;
		status = 5;
		isValid = 0;
		return(status);
	}

	//open the video file
	m = libvlc_media_new_path(libvlc, vidpath.str().c_str());
	if (m == NULL)
	{
		std::cerr << "Media path not valid." << std::endl;
		status = 6;
		isValid = 0;
		return(status);
	}
	//else
		//std::cerr << "Media path: " << m << std::endl;

    mp = libvlc_media_player_new_from_media(m);
	if (mp == NULL)
	{
		std::cerr << "Media Player not created." << std::endl;
		status = 7;
		isValid = 0;
		return(status);
	}

    libvlc_media_release(m);

	libvlc_video_set_scale(mp,1.5);		

    libvlc_video_set_callbacks(mp, lock, unlock, display, &context);

    libvlc_video_set_format(mp, "RV16", w, h, w*2);

	mpevent = libvlc_media_player_event_manager(mp);
	ResetStatus();
	GetStatus();

	Invisible();  //make the window invisible until we need it

	//flag that we tried to load
	requestload = 0;

	if (status != 0)
		isValid = 0;
	else
		isValid = 1;

	return(status);

}



//function to control/update the state machine
int Video::Update()
{

	int status = 0;

	//if the video isn't valid, don't do anything!
	if (!isValid)
	{
		if (requestload == 1)
			state = Load;
		else
			state = Idle;

		return(-1);
	}
	
	GetStatus();

	//figure out what state we are in and do accordingly
	switch(state)
	{
		case Load:
			
			//this is just an empy state to wait until the video has finished loading

			if (requestload == 0)
			{
				std::cerr << ">>Leaving VidLoad state to VidIdle state." << std::endl;
				state = Idle;
			}

			break; //end load state

		case Idle:
			//we sit here until a request to play comes in

			//request to play
			if (requestplay == 1)
			{
				//request to play has come in; try to play
				if (isVisible == 0)
					Visible();
				
				status = libvlc_media_player_play(mp);
				std::cerr << ">>VidIdle state: Play requested." << std::endl;

				if (status == 0)
					requestplay = 2;
			}
			else if (requestplay == 2)
			{
				
				if (mpstate == libvlc_Playing)
				{
					std::cerr << ">>Leaving VidIdle state to VidPlay state." << std::endl;
					state = Playing;
				}
			}

			//request to load a new video
			if (requestload == 1)
			{
				std::cerr << ">>Leaving VidIdle state to VidLoad state." << std::endl;
				state = Load;
			}

			//request rewind
			if (requestrewind == 1)
			{
				std::cerr << ">>Leaving VidIdle state to VidRewind state." << std::endl;
				state = Rewind;
			}

			if (requeststop == 1)
				requeststop = 0; //ignore this request since we are already stopped
			if (requestpause == 1)
				requestpause = 0; //ignore this request since we are already stopped


			break; //end Idle state

		case Playing:

			requestplay = 0;
			hasStarted = 1;

			//see if the state has changed; if so, obey this transition
			if (mpstate == libvlc_Stopped)
			{
				std::cerr << ">>Leaving VidPlay state to VidStopped state." << std::endl;
				state = Stopped;
			}
			else if (mpstate == libvlc_Ended)
			{
				std::cerr << ">>Leaving VidPlay state to VidEnded state." << std::endl;
				state = Ended;
			}
			else if (mpstate == libvlc_Paused)
			{
				std::cerr << ">>Leaving VidPlay state to VidPaused state." << std::endl;
				state = Paused;
			}
			else if (mpstate != libvlc_Playing)
			{
				std::cerr << ">>Invalid state, returning to Idle." << std::endl;
				state = Idle;
			}

			//if we haven't transitioned states, check if there are any transition requests
			if (requestpause == 1)
			{
				std::cerr << ">>VidPlay state: Pause requested." << std::endl;
				libvlc_media_player_pause(mp);

				requestpause = 2;
			}
			else if (requeststop == 1)
			{
				libvlc_media_player_stop(mp);
				std::cerr << ">>VidPlay state: Stop requested." << std::endl;
				requeststop = 2;
			}
			
			
			break; //end play state

		case Paused:

			requestpause = 0;
			
			//see if the state has changed; if so, obey this transition
			if (mpstate == libvlc_Stopped)
			{
				std::cerr << ">>Leaving VidPaused state to VidStopped state." << std::endl;
				state = Stopped;
			}
			else if (mpstate == libvlc_Ended)
			{
				std::cerr << ">>Leaving VidPaused state to VidEnded state." << std::endl;
				state = Ended;
			}
			else if (mpstate == libvlc_Playing)
			{
				std::cerr << ">>Leaving VidPaused state to VidPlay state." << std::endl;
				state = Playing;
			}
			else if (mpstate != libvlc_Paused)
			{
				std::cerr << ">>Invalid state, returning to Idle." << std::endl;
				state = Idle;
			}

			//if we haven't transitioned states, check if there are any transition requests
			if (requestplay == 1)
			{
				//request to play has come in; try to play
				if (isVisible == 0)
					Visible();
				
				std::cerr << ">>VidPaused state: Play requested." << std::endl;
				status = libvlc_media_player_play(mp);

				if (status == 0)
					requestplay = 2;
				else
					state = Idle;
			}
			else if (requeststop == 1)
			{
				std::cerr << ">>VidPaused state: Stop requested." << std::endl;
				libvlc_media_player_stop(mp);

				requeststop = 2;
			}
			else if (requestrewind == 1)
			{
				std::cerr << ">>VidPaused state: Rewind requested." << std::endl;
				state = Rewind;
			}
			

			break; //end paused state

		case Stopped:

			requeststop = 0;
			hasStopped = 1;

			//see if the state has changed; if so, obey this transition
			if (mpstate == libvlc_Ended)
			{
				std::cerr << ">>Leaving VidStopped state to VidEnded state." << std::endl;
				state = Ended;
			}
			else if (mpstate == libvlc_Playing)
			{
				std::cerr << ">>Leaving VidStopped state to VidPlay state." << std::endl;
				state = Playing;
			}
			else if (mpstate != libvlc_Stopped)
			{
				std::cerr << ">>Invalid state, returning to Idle." << std::endl;
				state = Idle;
			}

			if (requestplay==1)
			{
				//request to play has come in; try to play
				if (isVisible == 0)
					Visible();
				
				std::cerr << ">>VidStopped state: Play requested." << std::endl;
				status = libvlc_media_player_play(mp);
				
				if (status == 0)
					requestplay = 2;
				else
					state = Idle;
			}
			else if (requestpause == 1)
				requestpause = 0;	//we don't accept this request because we are not playing
			else if (requestrewind == 1)
			{
				std::cerr << ">>VidStopped state: Rewind requested." << std::endl;
				state = Rewind;
			}

			break; //end stopped state

		case Ended:

			hasEnded = 1;

			if (requestrewind == 1)
			{
				std::cerr << ">>Leaving VidEnded state to Rewind state." << std::endl;
				state = Rewind;
			}
			
			if (requestplay == 1)
			{
				std::cerr << ">>VidEnded state: Play requested. Rewinding first." << std::endl;

				//we must rewind first
				requestrewind = 1;
				state = Rewind;
			
				//we will keep the play request active so that when we get back to the Idle state we can start playing right away
				
			}

			//we will automatically proceed to rewind the video

			break; //end ended state

		case Rewind:

			if (isVisible == 1)
				Invisible();

			//to rewind we need to be in play state
			if (requestrewind == 1)
			{
				if (mpstate == libvlc_Ended)
				{
					std::cerr << ">> Rewind state = Ended; Stop requested." << std::endl;

					//we need to change the state to stopped first otherwise we cannot play
					libvlc_media_player_stop(mp);
					
					//if (mpstate = libvlc_Stopped)
					//	requestrewind = 2;
				}
				else if (mpstate == libvlc_Paused)
				{
					//if we are paused this is a "play" state, so we can just rewind without having to play first
					requestrewind = 2;
				}
				else //(mpstate == libvlc_Stopped)
				{
					std::cerr << ">> Rewind state = Stopped; Play requested." << std::endl;

					status = libvlc_media_player_play(mp);
				
					if (status == 0)
						requestrewind = 2;
				}
			}
			
			if (requestrewind == 2)
			{
				//now that we are playing we can set the position and stop - we need to do this quickly before we transition back to the Ended state
				libvlc_media_player_set_position(mp,0.0f);
				vidPos = libvlc_media_player_get_position(mp);
				std::cerr << ">>Video position: " << vidPos  << std::endl;  //<< " : " << libvlc_media_player_get_time(mp)
				if ((vidPos - 0.0f) < 1e-4)
					status = 1;
				
				requestrewind = 3;

			}
			else if (requestrewind == 3)
			{
				std::cerr << ">> Rewind state = Play/Pos Reset; Stop requested." << std::endl;
				libvlc_media_player_set_position(mp,0.0f);  //reset the position again just in case there was a long delay before we got baak here
				libvlc_media_player_stop(mp);
				
				ResetStatus();
				requestrewind = 0;
				state = Idle;
				std::cerr << ">>Leaving VidRewind state to VidIdle state." << std::endl;
			}
			
			break; //end rewind state

	} //end switch


}




void Video::SetPos(int x, int y)
{
	context.xpos = x;
	context.ypos = y;

}


void Video::SetValidStatus(int status)
{
	isValid = status;
}

int Video::IsValid()
{
	return(isValid);
}


int Video::GetStatus()
{

	float pos;

	if (!isValid)
		return(-1);

	//get the status of the video, and update some status flags
	mpstate = libvlc_media_get_state(m);

	//std::cerr << "  Video Play Status: " << mpstate << " : (" << hasStarted << "," << hasEnded << ")." << std::endl;
	
	return(mpstate);
}

int Video::HasStarted()
{
	if (!isValid)
		return(-1);

	return(hasStarted);
}

int Video::HasEnded()
{
	if (!isValid)
		return(-1);

	return(hasEnded);
}


int Video::HasStopped()
{
	if (!isValid)
		return(-1);

	return(hasStopped);
}

void Video::ResetStatus()
{
	hasStarted = 0;
	hasEnded = 0;
	hasStopped = 0;

	std::cerr << ">Vid status flags reset" << std::endl;
}


int Video::Play()
{
	//returns 0 if playback started.

	if (!isValid)
		return(-1);

	requestplay = 1;

	//std::cerr << "VidPlay State: " << libvlc_media_get_state(m) << std::endl;
	
	return(0);
}

int Video::Stop()
{
	int status = 0;

	if (!isValid)
		return(-1);

	requeststop = 1;

	return(0);
}

int Video::Pause()
{

	if (!isValid)
		return(-1);

	requestpause = 1;

	return(0);

}


int Video::ResetVid()
{
	if (!isValid)
		return(-1);

	requestrewind = 1;
		
	//std::cerr << "Video State: " << libvlc_media_get_state(m) << std::endl;

	return(0);

}


void Video::Visible()
{
	
	Uint32 winFlags;

	SDL_ShowWindow(context.window);
	winFlags = SDL_GetWindowFlags(context.window);
	if ((winFlags & SDL_WINDOW_SHOWN) ) //&& !isVisible
	{
		std::cerr << ">Vid window is visible." << std::endl;
		visTime = SDL_GetTicks();
		isVisible = 1;
		context.showVideo = 1;  //allow request to actually render video
	}
	
}

void Video::Invisible()
{
	Uint32 winFlags;

	SDL_HideWindow(context.window);
	winFlags = SDL_GetWindowFlags(context.window);
	if ((winFlags & SDL_WINDOW_HIDDEN) ) //&& isVisible != 0
	{
		std::cerr << ">Vid window is hidden." << std::endl;
		isVisible = 0;
		context.showVideo = 0;  //shut off request to actually render video
	}
	
}

int Video::VisibleState()
{
	return(isVisible);
}

void Video::CleanUp()
{
	if (isValid > 0)
	{
		
		// Stop stream and clean up libVLC.
		libvlc_media_player_stop(mp);
		libvlc_media_player_release(mp);
	}

	if(libvlc != NULL)
		libvlc_release(libvlc);

	// Close window and clean up libSDL.
    SDL_DestroyMutex(context.mutex);
    SDL_DestroyRenderer(context.renderer);
	SDL_DestroyWindow(context.window);

}
