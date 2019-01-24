#include "vlcVideoPlayer.h"


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




Video::Video(const char* fname, int x, int y, int w, int h) //SDL_Renderer *renderer, SDL_Texture *texture, SDL_mutex *mutex, SDL_Rect rect)
{
	
    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
	_putenv("VLC_PLUGIN_PATH=C:\\Users\\Coda\\Documents\\cpp code\\vlctest\\vlc-3.0.6\\plugins");
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

	//we will set up a dedicated window and renderer for the video, as it plays asynchronously
	context.window = SDL_CreateWindow(
            "Vidplayer",
            x-w/2,
            y-h/2,
            w, h,
            SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SKIP_TASKBAR); // 
    if (!context.window)
	{
        std::cerr << "Couldn't create window: " << SDL_GetError() << std::endl;
	}

    context.renderer = SDL_CreateRenderer(context.window, -1, 0);
    if (!context.renderer){
        std::cerr << "Couldn't create renderer: " << SDL_GetError() << std::endl;
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
    libvlc = libvlc_new(0, NULL);
    if(libvlc == NULL) {
		std::cerr << "LibVLC initialization failure." << std::endl;
		//exit;
        //return EXIT_FAILURE;
    }

	//set up the video file to be played
	std::stringstream vidpath;
	int d = 0;
	vidpath << VIDEOPATH << fname; //"\\Video" << d << ".divx";

	//open the video file
	m = libvlc_media_new_path(libvlc, vidpath.str().c_str());
	if (m == NULL)
		std::cerr << "Media path not valid." << std::endl;

    mp = libvlc_media_player_new_from_media(m);
	if (mp == NULL)
		std::cerr << "Media Player not created." << std::endl;

    libvlc_media_release(m);

	libvlc_video_set_scale(mp,1.5);		

    libvlc_video_set_callbacks(mp, lock, unlock, display, &context);

    libvlc_video_set_format(mp, "RV16", w, h, w*2);

	mpevent = libvlc_media_player_event_manager(mp);
	ResetStatus();
	GetStatus();

	context.showVideo = 0;

	Invisible();  //make the window invisible until we need it

}

void Video::SetPos(int x, int y)
{
	context.xpos = x;
	context.ypos = y;

}


int Video::GetStatus()
{
	//get the status of the video, and update some status flags
	mpstate = libvlc_media_get_state(m);

	if (hasStarted == 0 && (mpstate == libvlc_Playing))
	{
		hasStarted = 1;
		hasEnded = 0;
	}
	else if (hasStarted == 1 && (mpstate == libvlc_Stopped || mpstate == libvlc_Ended) )
	{
		hasEnded = 1;
		//hasStarted = 0; //we will not clear this until requested separately
	}

	//std::cerr << "  Video Play Status: " << mpstate << " : (" << hasStarted << "," << hasEnded << ")." << std::endl;

	return(mpstate);
}

int Video::HasStarted()
{
	GetStatus();

	return(hasStarted);
}

int Video::HasEnded()
{

	GetStatus();

	return(hasEnded);
}


void Video::ResetStatus()
{
	hasStarted = 0;
	hasEnded = 0;
}


int Video::Play()
{
	//returns 0 if playback started.

	int status = 1;
	
	context.showVideo = 1;
	Visible();

	GetStatus();

	if (mpstate != libvlc_Playing)
	{
		//start playing the video
		status = libvlc_media_player_play(mp);
		std::cerr << "Play video." << std::endl;

		//VidIsPlaying = 1;
		//libvlc_event_attach(mpevent,libvlc_MediaPlayerEndReached,videoEnded,VidIsPlaying); //set up a callback to detect video end

		GetStatus();

	}
	else
		status = 1;

	return(status);
}

int Video::Stop()
{
	int status = 1;

	context.showVideo = 0;
	Invisible();

	libvlc_media_player_stop(mp);
	std::cerr << "Request stop video." << std::endl;

	GetStatus();
	if ((mpstate == libvlc_Ended) || (mpstate == libvlc_Stopped))
	{
		status = 0;
	}

	return(status);
}

int Video::Pause()
{

	int status = 0;

	libvlc_media_player_pause(mp);
	std::cerr << "Request pause video." << std::endl;

	GetStatus();
	if (mpstate == libvlc_Paused)
		status = 1;

	return(status);

}

int Video::ResetVid()
{
	int status = 0;
	
	libvlc_media_player_play(mp);
	libvlc_media_player_set_position(mp,0.0f);
	libvlc_media_player_set_time(mp,0);
	float pos = libvlc_media_player_get_position(mp);
	std::cerr << "Video position: " << pos << " : " << libvlc_media_player_get_time(mp) << std::endl;
	if (pos == 0.0f)
		status = 1;
	libvlc_media_player_stop(mp);

	return(status);
}


void Video::Visible()
{
	SDL_ShowWindow(context.window);
}

void Video::Invisible()
{
	SDL_HideWindow(context.window);
}



void Video::CleanUp()
{
	// Stop stream and clean up libVLC.
    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(libvlc);

	// Close window and clean up libSDL.
    SDL_DestroyMutex(context.mutex);
    SDL_DestroyRenderer(context.renderer);
	SDL_DestroyWindow(context.window);

}



/*
//callback to detect when the video has stopped playing
int VidIsPlaying;
void videoEnded(const libvlc_event_t *event, void *vidPlaying)
{
	std::cerr << "VideoEnd callback (" << vidPlaying << "):(";
	*vidPlaying = 0;
	std::cerr << vidPlaying << "):  ";

}
*/
