#include <cmath>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>
#include <windows.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
//#include "bird.h"
#include "TrackBird.h"
//#include "BirdInput.h"
//#include "MouseInput.h"
#include "DataWriter.h"
#include "Object2D.h"

#include <gl/GL.h>
#include <gl/GLU.h>
/*
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "SDL_mixer.lib")
#pragma comment(lib, "SDL_ttf.lib")
#pragma comment(lib, "SDL_image.lib")
#pragma comment(lib, "Bird.lib")
*/
#pragma push(1)


/* 
	Sequence learning paradigm, with prediction allowed
	   -Modified from KineReachDemo code (Promit and Aneesh), 9/10/2012
	   -Originally written to perform the sequence learning task where the initial position is prompted, 
		   but the remaining targets in the sequence are not.
	   -General functionality includes loading a paradigm from a trial table, reading in trace files, and executing the experiment
	   -Data-saving functionality expanded to include saving target position and other display information

	   -System calibration last performed 9/18/2012 by AW

	   -Modifications, last updated 10/23/2012, AW:
	        -Game-loop state machine changed to include a waitITI state
			-DataWriter: added a velocity column; although this writes to all "birds", this only tracks velocity of the "player" (active bird only)
			-Image: allowed to draw left- or right-justified images with new function DrawAlign
			-Config: added velocity threshold values
	   -Velocity feedback included (10/23/2012 AW), using a velocity bar and predefined velocity parameters in config.h
	   -Increase priority of the process.

	   -Fixed set system config problem, so flock of birds sample rate is actually getting set.  11/12/2012, AW
	   
*/


//state machine
enum GameState
{
	Idle = 0x01,     //00001
	Finished = 0x10  //10000
};



SDL_Event event;
SDL_Surface* screen = NULL;
/* COM ports where the Flock of Birds are connected. The first element is always
 * 0, and subsequent elements are the port numbers in order.
 */
//WORD COM_port[5] = {0, 5,6,7,8};  //these need to match the ports that Windows has detected (see notes, AW for how to set up)
//InputDevice* controller[BIRDCOUNT + 1];
Object2D* cursor[BIRDCOUNT + 1];
//Object2D* player = NULL;
Object2D* trace;
TTF_Font* font = NULL;
SDL_Color textColor = {0, 0, 0};
DataWriter* writer = NULL;
GameState state;
Uint32 gameTimer;
//bool birds_connected;

//BIRDSYSTEMCONFIG sysconfig;
int trackstatus;
TrackSYSCONFIG sysconfig;
TrackDATAFRAME dataframe;
Uint32 DataStartTime = 0;


// Initializes everything and returns true if there were no errors
bool init();
// Sets up OpenGL
void setup_opengl();
// Performs closing operations
void clean_up();
// Draws objects on the screen
void draw_screen();

// Update loop (state machine)
void game_update();


int main(int argc, char* args[])
{
	int a = 0;

	std::cerr << "Start main." << std::endl;


	SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);
	//HIGH_PRIORITY_CLASS
	std::cerr << "Promote process priority to Above Normal." << std::endl;

	if (!init())
	{
		// There was an error during initialization
		std::cerr << "Initialization error." << std::endl;
		return 1;
	}
	
	bool quit = false;

	DataStartTime = SDL_GetTicks();

	while (!quit)
	{
		//game_update(); // Run the game loop

		int inputs_updated = 0;

		// Retrieve Flock of Birds data
		if (trackstatus>0)
		{
			// Update inputs from Ascension
			inputs_updated = TrackBird::GetUpdatedSample(&sysconfig,&dataframe);
		}

		// Handle SDL events
		while (SDL_PollEvent(&event))
		{
			// See http://www.libsdl.org/docs/html/sdlevent.html for list of event types
			if (event.type == SDL_MOUSEMOTION)
			{
				inputs_updated += ((trackstatus>0) ? 0 : 1) ; // Record data if birds are not connected
				//MouseInput::ProcessEvent(event); // Pass this event to the MouseInput class to process
				dataframe.x[0] = (GLfloat)event.motion.x * PHYSICAL_RATIO;
				dataframe.y[0] = (GLfloat)(SCREEN_HEIGHT - event.motion.y) * PHYSICAL_RATIO;
				dataframe.z[0] = 0.0f;
				dataframe.time[0] = SDL_GetTicks();
				dataframe.etime[0] = dataframe.time[0]-DataStartTime;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				// See http://www.libsdl.org/docs/html/sdlkey.html for Keysym definitions
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
			}
			else if (event.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		if ((state == Finished) && (SDL_GetTicks() - gameTimer >= 10000))
			quit = true;

		// Get data from input devices
		if (inputs_updated > 0) // if there is a new frame of data
		{

			for (int a = ((trackstatus>0) ? 1 : 0); a <= ((trackstatus>0) ? BIRDCOUNT : 0); a++)
			{
				InputFrame i;
				i.time = dataframe.time[a];
				i.x = dataframe.x[a];
				i.y = dataframe.y[a];
				i.z = dataframe.z[a];
				for (int b = 0; b < 3; b++)
					for (int c = 0; c < 3; c++)
						i.angmatrix[b][c] = dataframe.anglematrix[a][b][c];
				
				writer->Record(a, i);
				
				cursor[a]->SetPos(dataframe.x[a],dataframe.y[a]);
				//cursor[a]->SetAngle(dataframe.theta[a]);

			}

		}

		game_update(); // Run the game loop (state machine update)

		//if (updatedisplay)  //reduce number of calls to draw_screen -- does this speed up display/update?
		draw_screen();

	}

	clean_up();
	//return 0;
}


//initialization function - set up the experimental environment and load all relevant parameters/files
bool init()
{
	// Initialize Flock of Birds
	/* The program will run differently if the birds fail to initialize, so we
	 * store it in a bool.
	 */

	int a;
	char tmpstr[80];
	int b;

	//std::cerr << "Start init." << std::endl;

	trackstatus = TrackBird::InitializeBird(&sysconfig);
	if (trackstatus <= 0)
		std::cerr << "Tracker failed to initialize. Mouse Mode.";

	// Initialize SDL, OpenGL, SDL_mixer, and SDL_ttf
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		std::cerr << "SDL failed to intialize."  << std::endl;
		return false;
	}
	else
		std::cerr << "SDL initialized." << std::endl;

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
		SDL_OPENGL | (WINDOWED ? 0 : SDL_FULLSCREEN));
	if (screen == NULL)
	{
		std::cerr << "Screen failed to build." << std::endl;
		return false;
	}
	else
		std::cerr << "Screen built." << std::endl;

	setup_opengl();
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	if (TTF_Init() == -1)
	{
		std::cerr << "Audio failed to initialize." << std::endl;
		return false;
	}
	else
		std::cerr << "Audio initialized." << std::endl;

	//turn off the computer cursor
	SDL_ShowCursor(0);


	// Load files and initialize pointers
	font = TTF_OpenFont("Resources/arial.ttf", 28);
	Image* crosshair = Image::LoadFromFile("Resources/ctgt1.png");
	if (crosshair == NULL)
		std::cerr << "Image crosshair did not load." << std::endl;

	Image* tgttrace;  //is there a limit to the size of this array (stack limit?).  cannot seem to load more than 10 image traces...

	//load all the trace files
	sprintf(tmpstr,"%s/Trace%d.png",TRACEPATH,0);
	tgttrace = Image::LoadFromFile(tmpstr);
	if (tgttrace == NULL)
		std::cerr << "Image Trace" << a << " did not load." << std::endl;
	else
	{
		trace = new Object2D(tgttrace);
		trace->SetPos(PHYSICAL_WIDTH/2,PHYSICAL_HEIGHT/2);
		std::cerr << "   Trace0 loaded." << std::endl;
	}

	std::cerr << "Images loaded." << std::endl;


	//assign the data-output file name based on the trial-table name 
	char savfile[15] = "calib_data.txt";

	std::cerr << "SavFileName: " << savfile << std::endl;

	writer = new DataWriter(&sysconfig,savfile);  //set up the data-output file

	// Assign array index 0 of controller and cursor to correspond to mouse control
	//controller[0] = new MouseInput();
	if (trackstatus > 0)
	{
		/* Assign birds to the same indices of controller and cursor that they use
		 * for the Flock of Birds
		 */

		for (a = 1; a <= BIRDCOUNT; a++)
		{
			//controller[a] = new BirdInput(a);
			cursor[a] = new Object2D(crosshair);
			//player = cursor[HAND];
		}
	}
	else
	{
		// Use mouse control
		cursor[0] = new Object2D(crosshair);
		//player = cursor[0];
	}	

	/* To create text, call a render function from SDL_ttf and use it to create
	 * an Image object. See http://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf.html#SEC42
	 * for a list of render functions.
	 */

	SDL_WM_SetCaption("Calibration", NULL);

	// Set the initial game state
	state = Idle; 
	gameTimer = SDL_GetTicks();

	std::cerr << "initialization complete." << std::endl;
	return true;
}


static void setup_opengl()
{
	glClearColor(255, 255, 255, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* The default coordinate system has (0, 0) at the bottom left. Width and
	 * height are in meters, defined by PHYSICAL_WIDTH and PHYSICAL_HEIGHT
	 * (config.h). If MIRRORED (config.h) is set to true, everything is flipped
	 * horizontally.
	 */
	glOrtho(MIRRORED ? PHYSICAL_WIDTH : 0, MIRRORED ? 0 : PHYSICAL_WIDTH,
		0, PHYSICAL_HEIGHT, -1.0f, 1.0f);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


//end the program; clean up everything neatly.
void clean_up()
{
	for (int a = 0; a <= ((trackstatus>0) ? BIRDCOUNT : 0); a++)
	{
		delete cursor[a];
	}
	
	delete trace;
	
	delete writer;
	Mix_CloseAudio();
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	if (trackstatus > 0)
		TrackBird::ShutDownBird(&sysconfig);
}

//control what is drawn to the screen
static void draw_screen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	// Draw the mouse cursor, as well as all the birds if available
	for (int a = 0; a <= (birds_connected ? BIRDCOUNT : 0); a++)
	{
		cursor[a]->Draw();
	}
	*/

	//draw the trace specified
	trace->Draw();


	//draw only the mouse/birds requested, as initialized in init()
	if (trackstatus>0)
	{
		cursor[1]->Draw(CURSOR_RADIUS * 2, CURSOR_RADIUS * 2);
		cursor[3]->Draw(CURSOR_RADIUS * 2, CURSOR_RADIUS * 2);
	}
	else
	{
		cursor[0]->Draw(CURSOR_RADIUS * 2, CURSOR_RADIUS * 2);
	}


	SDL_GL_SwapBuffers();
	glFlush();

	//updatedisplay = false;
}

//game update loop - state machine controlling the status of the experiment
void game_update()
{

	switch (state)
	{
		case Idle:
			/* If player starts hovering over start marker, set state to Starting
			 * and store the time -- this is for trial #1 only!
			 */
			if (SDL_GetTicks() - gameTimer > CALIBTIME)
				state = Finished;

			break;
		case Finished:
			// Trial table ended, wait for program to quit


			break;
			


	}
}

