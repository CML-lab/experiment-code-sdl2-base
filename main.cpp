#include <cmath>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>
#include <windows.h>
#include <vector>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include "FTDI.h"

#include "MouseInput.h"
#include "TrackBird.h"

#include "Circle.h"
#include "DataWriter.h"
#include "HandCursor.h"
#include "Object2D.h"
#include "Path2D.h"
#include "Region2D.h"
#include "Sound.h"
#include "SpeedBar.h"
#include "Timer.h"
#include "Image.h"

#include "config.h"

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
#pragma comment(lib, "ftd2xx.lib")
#pragma comment(lib, "ATC3DG.lib")
*/
#pragma push(1)

//state machine
enum GameState
{
	Idle = 0x01,       //00001
	Starting = 0x03,   //00011
	Active = 0x06,     //00110
	ShowResult = 0x08, //01000
	Finished = 0x10    //10000
};



SDL_Event event;
int nWindows;
SCREEN_struct screens[NSCREEN];
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

//structure to control which screens a given texture is drawn to (for Object2D and text objects)
typedef struct {
	int drawtraces[NSCREEN];
	int drawmaintext[NSCREEN];
	int drawsecondtext[NSCREEN];
	int drawsubtext[NSCREEN];
} DRAWSTRUC;

DRAWSTRUC drawstruc;


HandCursor* curs[BIRDCOUNT + 1];
HandCursor* player = NULL;
Circle* startCircle = NULL;
Circle* targCircle = NULL;
Circle* photosensorCircle = NULL;
Region2D barrierRegions[NREGIONS];
Path2D barrierPaths[NPATHS];
Object2D* traces[NTRACES];
Image* text = NULL;
//Image* trialnum = NULL;
Image* textsubwin = NULL;
Sound* startbeep = NULL;
Sound* scorebeep = NULL;
Sound* errorbeep = NULL;
SDL_Color textColor = {0, 0, 0, 1};
SDL_Color whitetextColor = {1, 1, 1, 1};
DataWriter* writer = NULL;
GameState state;
Timer* trialTimer;
Timer* hoverTimer;
Timer* movTimer;

typedef struct {
	Image* title;
	Image* trialnum;
	Image* statusflagpath;
	Image* statusflagregion;
	Image* statusflagtgt;
} TRIALTEXT;
TRIALTEXT trialtext;



//Uint32 gameTimer;
//Uint32 hoverTimer;

//velocity-tracking variables
float PeakVel;
SpeedBar velBar;

//Photosensor (FTDI) variables
FT_HANDLE ftHandle;
bool sensorsActive;

//tracker variables
int trackstatus;
TrackSYSCONFIG sysconfig;
TrackDATAFRAME dataframe[BIRDCOUNT+1];
Uint32 DataStartTime = 0;

//variables to compute the earned score
int score = 0;

//colors
float redColor[3] = {1.0f, 0.0f, 0.0f};
float greenColor[3] = {0.0f, 1.0f, 0.0f};
float blueColor[3] = {0.0f, 0.0f, 1.0f};
float cyanColor[3] = {0.0f, 0.5f, 1.0f};
float grayColor[3] = {0.6f, 0.6f, 0.6f};
float blkColor[3] = {0.0f, 0.0f, 0.0f};
float whiteColor[3] = {1.0f, 1.0f, 1.0f};
float orangeColor[3] = {1.0f, 0.5f, 0.0f};
float *startColor = greenColor;
float *targColor = blueColor;
float *targHitColor = cyanColor;
float *cursColor = redColor;



// Trial table structure, to keep track of all the parameters for each trial of the experiment
typedef struct {
	//int TrialType;		// Flag 1-for trial type
	float startx,starty;	// x/y pos of start target; also, trace will be centered here!
	float xpos,ypos;		// x/y pos of target.
	int path;
	int region;
	int trace;
	int iti;				//inter-trial interval
} TRTBL;

#define TRTBL_SIZE 1000
TRTBL trtbl [TRTBL_SIZE];

int NTRIALS = 0;
int CurTrial = 0;

#define curtr trtbl[CurTrial]


//target structure; keep track of the target and other parameters, for writing out to data stream
TargetFrame Target;

// Initializes everything and returns true if there were no errors
bool init();
// Sets up OpenGL
void setup_opengl();
// Performs closing operations
void clean_up();
// Specifies the objects that are to be drawn to the primary screen
void draw_screen(int win);
//Specifies what text objects are to be drawn to the experimenter screen
void draw_experimenter_text();
//wrapper around draw_screen() to handle secondary displays
void draw_all_screens();
//file to load in trial table
int LoadTrFile(char *filename);
// Update loop (state machine)
void game_update();

bool quit = false;  //flag to cue exit of program



int main(int argc, char* args[])
{
	int a = 0;

	//redirect stderr output to a file
	freopen( "./Debug/errorlog.txt", "w", stderr); 

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
	
	DataStartTime = SDL_GetTicks();

	while (!quit)
	{
		int inputs_updated = 0;

		// Retrieve Flock of Birds data
		if (trackstatus>0)
		{
			// Update inputs from Flock of Birds
			inputs_updated = TrackBird::GetUpdatedSample(&sysconfig,dataframe);
		}

		// Handle SDL events
		while (SDL_PollEvent(&event))
		{
			// See http://www.libsdl.org/docs/html/sdlevent.html for list of event types
			if (event.type == SDL_MOUSEMOTION)
			{
				if (trackstatus <= 0)
				{
					MouseInput::ProcessEvent(event);
					inputs_updated = MouseInput::GetFrame(dataframe);

				}
			}
			else if (event.type == SDL_KEYDOWN)
			{
				// See http://www.libsdl.org/docs/html/sdlkey.html for Keysym definitions
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else //if( event.key.keysym.unicode < 0x80 && event.key.keysym.unicode > 0 )
				{
					Target.key = *SDL_GetKeyName(event.key.keysym.sym);  //(char)event.key.keysym.unicode;
					//std::cerr << Target.flag << std::endl;
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				Target.key = '0';
			}
			else if (event.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		if ((CurTrial >= NTRIALS) && (state == Finished) && (trialTimer->Elapsed() >= 10000))
			quit = true;

		// Get data from input devices
		if (inputs_updated > 0) // if there is a new frame of data
		{

			if (sensorsActive)
			{
				UCHAR bit = 4;  //this is the CTS line
				Target.PSstatus = Ftdi::GetFtdiBitBang(ftHandle,bit);
			}
			else
				Target.PSstatus = -99;

			//updatedisplay = true;
			for (int a = ((trackstatus>0) ? 1 : 0); a <= ((trackstatus>0) ? BIRDCOUNT : 0); a++)
			{
				if (dataframe[a].ValidInput)
				{
					curs[a]->UpdatePos(dataframe[a].x,dataframe[a].y);
					dataframe[a].vel = curs[a]->GetVel();
					writer->Record(a, dataframe[a], Target);
				}
			}

		}

		game_update(); // Run the game loop (state machine update)

		//if (updatedisplay)  //reduce number of calls to draw_screen -- does this speed up display/update?
		draw_all_screens();

	}

	clean_up();
	return 0;
}



//function to read in the name of the trial table file, and then load that trial table
int LoadTrFile(char *fname)
{

	//std::cerr << "LoadTrFile begin." << std::endl;

	char tmpline[100] = ""; 
	int ntrials = 0;

	//read in the trial file name
	std::ifstream trfile(fname);

	if (!trfile)
	{
		std::cerr << "Cannot open input file." << std::endl;
		return(-1);
	}
	else
		std::cerr << "Opened TrialFile " << TRIALFILE << std::endl;

	trfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the trial-table file

	while(!trfile.eof())
	{
		sscanf(tmpline, "%f %f %f %f %d %d %d %d", 
			&trtbl[ntrials].startx,&trtbl[ntrials].starty,
			&trtbl[ntrials].xpos,&trtbl[ntrials].ypos,
			&trtbl[ntrials].path,
			&trtbl[ntrials].region,
			&trtbl[ntrials].trace, 
			&trtbl[ntrials].iti);

			ntrials++;
			trfile.getline(tmpline,sizeof(tmpline),'\n');
	}

	trfile.close();
	if(ntrials == 0)
	{
		std::cerr << "Empty input file." << std::endl;
		//exit(1);
		return(-1);
	}
	return ntrials;
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
	char fname[50] = TRIALFILE;
	//char dataPath[50] = DATA_OUTPUT_PATH;

	//initialize draw structure parameters
	for (a = 0; a < 3; a++)
	{
		drawstruc.drawtraces[a] = 0;
		drawstruc.drawmaintext[a] = 0;
		drawstruc.drawsecondtext[a] = 0;
		//drawstruc.drawsubtext[a] = 0;
	}
	drawstruc.drawtraces[0] = 1;
	drawstruc.drawmaintext[0] = 1;
	drawstruc.drawsecondtext[1] = 1;
	//drawstruc.drawsubtext[1] = 1;

	//std::cerr << "Start init." << std::endl;

	std::cerr << std::endl;

	trackstatus = TrackBird::InitializeBird(&sysconfig);
	if (trackstatus <= 0)
		std::cerr << "Tracker failed to initialize. Mouse Mode." << std::endl;

	std::cerr << std::endl;

	// Initialize SDL, OpenGL, SDL_mixer, and SDL_ttf
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "SDL failed to intialize."  << std::endl;
		return false;
	}
	else
		std::cerr << "SDL initialized." << std::endl;

	//initialize all the screens
	nWindows = SDL_GetNumVideoDisplays();
	std::cerr << "Number of displays: " << nWindows << std::endl;

	/*We always want the experiment screen to be in position 0 in the matrix, but it will refer to
	 * either the primary screen (one screen) or secondary screen (dual-screen) depending on the setup.
	*/
	if (nWindows == 1)
	{
		screens[0].displayIndex = 0;

		//primary screen
        SDL_GetDisplayBounds(0 , &screens[0].bounds );

		//define some constants based on the dimensions of the primary window
		SCREEN_WIDTH = screens[0].bounds.w;
		SCREEN_HEIGHT = screens[0].bounds.h;

		std::cerr << "   Display" << 0 << ": (" << screens[0].bounds.x << ',' << screens[0].bounds.y << "), " << screens[0].bounds.w << "x" << screens[0].bounds.h << std::endl;
        screens[0].window = SDL_CreateWindow
            ( 
            "Display", 
            screens[0].bounds.x, 0, 
            screens[0].bounds.w, screens[0].bounds.h, 
            SDL_WINDOW_OPENGL | (WINDOWED ? 0 : SDL_WINDOW_BORDERLESS) //SDL_WINDOW_FULLSCREEN 
            );
        //SDL_ShowWindow( screen.window );
		if (screens[0].window == NULL)
		{
			std::cerr << "Screen " << 0 << " failed to build." << std::endl;
			return false;
		}
		else
		{
			screens[0].glcontext = SDL_GL_CreateContext(screens[0].window);
			std::cerr << "Screen  " << 0 << " built." << std::endl;
		}

	}
	else
	{
		screens[0].displayIndex = 1;
		screens[1].displayIndex = 0;

		//save bounds from secondary monitor to primary display
		SDL_GetDisplayBounds( screens[1].displayIndex, &screens[1].bounds );

		//save bounds from primary monitor to secondary display
		SDL_GetDisplayBounds( screens[0].displayIndex, &screens[0].bounds );

		//define some constants based on the dimensions of the primary window
		SCREEN_WIDTH = screens[0].bounds.w;
		SCREEN_HEIGHT = screens[0].bounds.h;

		for (a = 0; a < 2; a++)
		{

			std::cerr << "   Display" << a << ": (" << screens[a].bounds.x << ',' << screens[a].bounds.y << "), " << screens[a].bounds.w << "x" << screens[a].bounds.h << std::endl;
			screens[a].window = SDL_CreateWindow
				( 
				"Display", 
				screens[a].bounds.x, 0, 
				screens[a].bounds.w, screens[a].bounds.h, 
				SDL_WINDOW_OPENGL | (WINDOWED ? 0 : SDL_WINDOW_BORDERLESS) //SDL_WINDOW_FULLSCREEN 
				);
			//SDL_ShowWindow( screen.window );
			if (screens[a].window == NULL)
			{
				std::cerr << "Screen " << a << " failed to build." << std::endl;
				return false;
			}
			else
			{
				screens[a].glcontext = SDL_GL_CreateContext(screens[a].window);
				std::cerr << "Screen  " << a << " built." << std::endl;
			}
		}

		//set the appropriate texture draw flags for a secondary display
		drawstruc.drawtraces[1] = 1;  //also draw onto the subwindow
		drawstruc.drawmaintext[1] = 1; //also draw onto the subwindow
		drawstruc.drawsecondtext[1] = 1; //draw secondary text to the experimenter window


		/*
		 * The code below sets up a subwindow to mirror the display into. This solution works, but behaves oddly
		 * on the kinereach computer and also incurs some delays, potentially because of having to do so many draws.
		 * To address these issues, we will use a more crude method of just making the secondary display window larger 
		 * to give us a border region for writing text
		 * /


		//set up the sub-window that will be a mirror of the original window; this will always be the last window and will be situated on the second window
		screens[2].bounds.x = screens[1].bounds.x;
		screens[2].bounds.y = screens[1].bounds.y;
		screens[2].bounds.w = screens[0].bounds.w;
		screens[2].bounds.h = screens[0].bounds.h;
		std::cerr << "   Subwindow: (" << screens[2].bounds.x << ',' << screens[2].bounds.y << "), " << screens[2].bounds.w << "x" << screens[2].bounds.h << std::endl;
		screens[2].window = SDL_CreateWindow
			( 
			"Subwindow-Mirror", 
			screens[2].bounds.x, 0, 
			screens[2].bounds.w*0.75, screens[2].bounds.h*0.75, 
			SDL_WINDOW_OPENGL | (WINDOWED ? 0 : SDL_WINDOW_BORDERLESS)
			);
		if (screens[2].window == NULL)
		{
			std::cerr << "Subwindow failed to build." << std::endl;
			return false;
		}
		else
		{
			screens[2].glcontext = SDL_GL_CreateContext(screens[2].window);
			std::cerr << "Subwindow built." << std::endl;
		}
		*/
	}

	SDL_GL_SetSwapInterval(0); //ask for immediate updates rather than syncing to vertical retrace

	setup_opengl();

	a = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 512);  //initialize SDL_mixer
	if (a != 0)
	{
		std::cerr << "Audio failed to initialize." << std::endl;
		return false;
	}
	else
		std::cerr << "Audio initialized." << std::endl;


	//initialize SDL_TTF (text handling)
	if (TTF_Init() == -1)
	{
		std::cerr << "SDL_TTF failed to initialize." << std::endl;
		return false;
	}
	else
		std::cerr << "SDL_TTF initialized." << std::endl;

	//turn off the computer cursor
	SDL_ShowCursor(0);

	std::cerr << std::endl;

	// Load files and initialize pointers
	
	//load all the trace files
	/* Note, for textures the draw specification is annoying: you have to draw a texture into each window/context that you want.
	 * To that end, it is necessary to specify an array indictating that images should be drawn both to the main window and, if
	 * available, the mirrored subwindow for the experimenter
	*/
	Image* tgttraces[NTRACES];

	for (a = 0; a < NTRACES; a++)
	{
		sprintf(tmpstr,"%s/Trace%d.png",TRACEPATH,a);
		tgttraces[a] = Image::LoadFromFile(tmpstr, drawstruc.drawtraces);
		if (tgttraces[a] == NULL)
			std::cerr << "Image Trace" << a << " did not load." << std::endl;
		else
		{
			traces[a] = new Object2D(tgttraces[a]);
			std::cerr << "   Trace " << a << " loaded." << std::endl;
			traces[a]->SetPos(PHYSICAL_WIDTH / 2, PHYSICAL_HEIGHT / 2);
		}
	}
	

	//load the path files
	for (a = 0; a < NPATHS; a++)
	{
		sprintf(tmpstr,"%s/Path%d.txt",PATHPATH,a); 
		barrierPaths[a] = Path2D::LoadPathFromFile(tmpstr);
		if (barrierPaths[a].GetPathNVerts() < 0)
			std::cerr << "   Path " << a << " did not load." << std::endl;
		else
			std::cerr << "   Path " << a << " loaded." << std::endl;
	}

	//load the region files
	for (a = 0; a < NREGIONS; a++)
	{
		sprintf(tmpstr,"%s/Region%d.txt",REGIONPATH,a); 
		barrierRegions[a] = Region2D::LoadRegionFromFile(tmpstr);
		if (barrierRegions[a].GetRegionSides() <= 2)
			std::cerr << "   Region " << a << " did not load." << std::endl;
		else
			std::cerr << "   Region " << a << " loaded." << std::endl;
	}

	//initialize the speed bar object
	//velBar.MakeSpeedBar(PHYSICAL_WIDTH/2, PHYSICAL_HEIGHT*3/4, 0.2, 0.02, 0.0, 2.0, 0.5, 1.5,'h');
	velBar.MakeSpeedBar(PHYSICAL_WIDTH*3/4, PHYSICAL_HEIGHT/2, 0.02, 0.2, 0.0, 2.0, 0.5, 1.5,'v');
	velBar.On();
		
	//std::cerr << "Images loaded: " << a-1 << "." << std::endl;


	startCircle = new Circle(curtr.startx, curtr.starty, START_RADIUS*2, startColor);
	startCircle->SetBorderWidth(0.001f);
	startCircle->SetBorderColor(blkColor);
	startCircle->On();
	startCircle->BorderOn();

	targCircle = new Circle(curtr.startx+curtr.xpos, curtr.starty+curtr.ypos, TARGET_RADIUS*2, startColor);
	targCircle->SetBorderColor(blkColor);
	targCircle->SetBorderWidth(0.002f);
	targCircle->BorderOn();
	targCircle->Off();
	
	photosensorCircle = new Circle(0.0f,0.24,0.05,blkColor);
	photosensorCircle->SetBorderColor(blkColor);
	photosensorCircle->BorderOff();
	photosensorCircle->On();
	
	//initialize the photosensor
	int status = -5;
	int devNum = 0;

	UCHAR Mask = 0x0f;  
	//the bits in the upper nibble should be set to 1 to be output lines and 0 to be input lines (only used 
	//  in SetSensorBitBang() ). The bits in the lower nibble should be set to 1 initially to be active lines.

	status = Ftdi::InitFtdi(devNum,&ftHandle,1,Mask);
	std::cerr << "PhotoSensor: " << status << std::endl;

	Ftdi::SetFtdiBitBang(ftHandle,Mask,3,0);

	UCHAR dataBit;

	FT_GetBitMode(ftHandle, &dataBit);
	
	std::cerr << "DataByte: " << std::hex << dataBit << std::dec << std::endl;
	
	if (status==0)
	{
		printf("PhotoSensor found and opened.\n");
		sensorsActive = true;
	}
	else
	{
		if (status == 1)
			std::cerr << "   Failed to create device list." << std::endl;
		else if (status == 2)
			std::cerr << "   Sensor ID=" << devNum << " not found." << std::endl;
		else if (status == 3)
			std::cerr << "   Sensor " << devNum << " failed to open." << std::endl;
		else if (status == 4)
			std::cerr << "   Sensor " << devNum << " failed to start in BitBang mode." << std::endl;
		else
			std::cerr << "UNDEFINED ERROR!" << std::endl;

		sensorsActive = false;
	}

	std::cerr << std::endl;



	//load trial table from file
	NTRIALS = LoadTrFile(fname);
	//std::cerr << "Filename: " << fname << std::endl;
	
	if(NTRIALS == -1)
	{
		std::cerr << "Trial File did not load." << std::endl;
		return false;
	}
	else
		std::cerr << "Trial File loaded: " << NTRIALS << " trials found." << std::endl;

	//assign the data-output file name based on the trial-table name 
	std::string savfile;
	savfile.assign(fname);
	savfile.insert(savfile.rfind("."),"_data");

	std::strcpy(fname,savfile.c_str());

	std::cerr << "SavFileName: " << fname << std::endl;

	writer = new DataWriter(&sysconfig,fname);  //set up the data-output file


	// set up the cursors
	if (trackstatus > 0)
	{
		/* Assign birds to the same indices of controller and cursor that they use
		* for the Flock of Birds
		*/
		for (a = 1; a <= BIRDCOUNT; a++)
		{
			curs[a] = new HandCursor(curtr.startx, curtr.starty, CURSOR_RADIUS*2, cursColor);
			curs[a]->BorderOff();
			curs[a]->SetOrigin(curtr.startx, curtr.starty);
		}

		player = curs[HAND];  //this is the cursor that represents the hand

	}
	else
	{
		// Use mouse control
		curs[0] = new HandCursor(curtr.startx, curtr.starty, CURSOR_RADIUS*2, cursColor);
		curs[0]->SetOrigin(curtr.startx, curtr.starty);
		player = curs[0];
	}


	player->On();

	PeakVel = -1;


	//load sound files
	startbeep = new Sound("Resources/startbeep.wav");
	scorebeep = new Sound("Resources/coin.wav");
	errorbeep = new Sound("Resources/errorbeep1.wav");

	//set up placeholder text
	text = Image::ImageText(text, " ","arial.ttf", 28, textColor, drawstruc.drawmaintext);
	text->Off();

	//set up trial number text image
	//trialnum = Image::ImageText(trialnum,"1","arial.ttf", 12,textColor, drawstruc.drawmaintext);
	//trialnum->On();

	//setup experimenter display text
	trialtext.title = Image::ImageText(trialtext.title,"Trial Information","arial.ttf",20,textColor, drawstruc.drawsecondtext);
	std::stringstream texttn;
	texttn << "Trial 1 of " << NTRIALS;
	trialtext.trialnum = Image::ImageText(trialtext.trialnum,texttn.str().c_str(),"arial.ttf",15,textColor, drawstruc.drawsecondtext);
	trialtext.statusflagpath = Image::ImageText(trialtext.statusflagpath,"Hit Path: False","arial.ttf",15,textColor, drawstruc.drawsecondtext);
	trialtext.statusflagregion = Image::ImageText(trialtext.statusflagregion,"Hit Region: False","arial.ttf",15,textColor, drawstruc.drawsecondtext);
	trialtext.statusflagtgt = Image::ImageText(trialtext.statusflagtgt,"Hit Target: False","arial.ttf",15,textColor, drawstruc.drawsecondtext);

	textsubwin = Image::ImageText(textsubwin,"SubWindow","arial.ttf",28,textColor, drawstruc.drawsubtext);


	hoverTimer = new Timer();
	trialTimer = new Timer();
	movTimer = new Timer();
	
	// Set the initial game state
	state = Idle; 

	std::cerr << "initialization complete." << std::endl;
	return true;
}


static void setup_opengl()
{
	int a;

	
	//set up the primary display
	SDL_GL_MakeCurrent(screens[0].window,screens[0].glcontext);

	glClearColor(1, 1, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* The default coordinate system has (0, 0) at the bottom left. Width and
	* height are in meters, defined by PHYSICAL_WIDTH and PHYSICAL_HEIGHT
	* (config.h). If MIRRORED (config.h) is set to true, everything is flipped
	* horizontally.
	*/
	glOrtho(MIRRORED ? PHYSICAL_WIDTH : 0, MIRRORED ? 0 : PHYSICAL_WIDTH,
		0, PHYSICAL_HEIGHT, -1.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	

	//set up the secondary display if available
	if (nWindows > 1)
	{
		//set up the full window display
		SDL_GL_MakeCurrent(screens[1].window,screens[1].glcontext);

		glClearColor(0.5, 0.5, 0.5, 0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		/* This is a secondary display for the experimenter, but can't get screen coordinates to work for some reason so we will stay in physical coordinates.
		* So we will have (0 , 0) at the bottom left, and PHYSICAL_WIDTH and PHYSICAL_HEIGHT
		* in pixels. We will never mirror this display. 
		*/
		//glOrtho(0, screens[1].bounds.w, 0, screens[1].bounds.h, -1.0f, 1.0f);
		//glOrtho(0, PHYSICAL_WIDTH, 0, PHYSICAL_HEIGHT, -1.0f, 1.0f);
		glOrtho(0, PHYSICAL_WIDTH*1.25, 0, PHYSICAL_HEIGHT*1.25, -1.0f, 1.0f);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);


		/*
		//set up the subwindow mirror
		SDL_GL_MakeCurrent(screens[2].window,screens[2].glcontext);

		glClearColor(1, 1, 1, 0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		/* This is a mirror of the original display screen, so it will have the same physical dimensions 
		* defined by PHYSICAL_WIDTH and PHYSICAL_HEIGHT (config.h). This will be exactly the mirror image of what is on screen, so 
		* it shows what the participant sees through the mirror. If the mirrored flag is not set, this screen will appear backwards.
		* /
		glOrtho(MIRRORED ? 0 : PHYSICAL_WIDTH, MIRRORED ? PHYSICAL_WIDTH : 0,
			0, PHYSICAL_HEIGHT, -1.0f, 1.0f);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
		*/
	}


}


//end the program; clean up everything neatly.
void clean_up()
{
	int a;

	delete startCircle;
	delete targCircle;
	delete scorebeep;
	delete errorbeep;
	
	for (a = 0; a < NTRACES; a++)
		delete traces[a];
	

	int status = Ftdi::CloseFtdi(ftHandle,1);

	delete text;
	//delete trialnum;
	delete trialtext.title;
	delete trialtext.trialnum;

	delete writer;

	for(a = 0; a < nWindows; a++ )
    {
		SDL_GL_DeleteContext(screens[a].glcontext);
		SDL_DestroyWindow(screens[a].window);
	}

	Mix_CloseAudio();
	TTF_Quit();
	SDL_Quit();
	if (trackstatus > 0)
		TrackBird::ShutDownBird(&sysconfig);

	freopen( "CON", "w", stderr );

}


//wrapper to handle dual-display draws by calling draw_screen()
static void draw_all_screens()
{

	int a;


	SDL_GL_MakeCurrent(screens[0].window,screens[0].glcontext);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//call the main screen draw function
	draw_screen(0);

	SDL_GL_SwapWindow(screens[0].window);
	glFlush();


	if (nWindows > 1)
	{

		//draw experimenter information to the secondary display
		SDL_GL_MakeCurrent(screens[1].window,screens[1].glcontext);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//call the secondary screen draw function
		
		glColor3f(1.0f,1.0f,1.0f);
		glRectf(0.0f,0.0f,PHYSICAL_WIDTH,PHYSICAL_HEIGHT);

		draw_screen(1);
		draw_experimenter_text();
		
		SDL_GL_SwapWindow(screens[1].window);
		glFlush();

		
		//draw a copy of the primary scren to the subwindow
		SDL_GL_MakeCurrent(screens[2].window,screens[2].glcontext);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*
		//call the main screen draw function again to make a second copy of the screen to the subwindow
		draw_screen(2);

		//add in new stuff on the subwindow
		textsubwin->Draw(PHYSICAL_WIDTH/2,0.60, 2);

		SDL_GL_SwapWindow(screens[2].window);
		glFlush();
		*/
		

	}







}


static void draw_experimenter_text()
{

	trialtext.title->DrawAlign(.05f,PHYSICAL_HEIGHT*1.25f-0.05f,trialtext.title->GetWidth(),trialtext.title->GetHeight(), 3, 1);
	trialtext.trialnum->DrawAlign(.06f,PHYSICAL_HEIGHT*1.25f-0.07f,trialtext.trialnum->GetWidth(),trialtext.trialnum->GetHeight(), 3, 1);
	trialtext.statusflagtgt->DrawAlign(.06f,PHYSICAL_HEIGHT*1.25f-0.09f,trialtext.statusflagtgt->GetWidth(),trialtext.statusflagtgt->GetHeight(), 3, 1);
	trialtext.statusflagpath->DrawAlign(.06f,PHYSICAL_HEIGHT*1.25f-0.10f,trialtext.statusflagpath->GetWidth(),trialtext.statusflagpath->GetHeight(), 3, 1);
	trialtext.statusflagregion->DrawAlign(.06f,PHYSICAL_HEIGHT*1.25f-0.11f,trialtext.statusflagregion->GetWidth(),trialtext.statusflagregion->GetHeight(), 3, 1);

}


//control what is drawn to the screen
static void draw_screen(int win)
{

	//draw the trace specified
	Target.trace = -1;
	for (int a = 0; a < NTRACES; a++)
	{
		traces[a]->Draw(win);
		if (traces[a]->DrawState())
			Target.trace = a;
	}
	
	//draw the velocity feedback bar
	velBar.Draw();
	
	//draw the region
	Target.region = -1;
	for (int a = 0; a < NREGIONS; a++)
	{
		barrierRegions[a].Draw();
		if (barrierRegions[a].DrawState())
			Target.path = a;
	}

	//draw the path
	Target.path = -1;
	for (int a = 0; a < NPATHS; a++)
	{
		barrierPaths[a].Draw();
		if (barrierPaths[a].DrawState())
			Target.path = a;
	}

	// Draw the start marker, if true
	startCircle->Draw();
	if (startCircle->drawState())
	{
		Target.startx = startCircle->GetX();
		Target.starty = startCircle->GetY();
	}
	else
	{
		Target.startx = -100;
		Target.starty = -100;
	}


	// Draw the target marker for the current trial, if true
	targCircle->Draw();
	if (targCircle->drawState())
	{
		// Marker is stretched to the activation radius
		Target.tgtx = targCircle->GetX();
		Target.tgty = targCircle->GetY();
	}
	else
	{
		Target.tgtx = -100;
		Target.tgty = -100;
	}

	photosensorCircle->Draw();

	player->Draw();


	// Draw text - provide feedback at the end of the block
	text->Draw(0.6f, 0.65f, win);

	//write the trial number
	//trialnum->Draw(PHYSICAL_WIDTH*23/24, PHYSICAL_HEIGHT*23/24, win);



}


//game update loop - state machine controlling the status of the experiment
bool mvtStarted = false;

bool reachedvelmin = false;
bool reachedvelmax = false;

bool mvmtEnded = false;
bool hitTarget = false;
bool hitRegion = false;
bool hitPath = false;

float LastPeakVel = 0;
bool returntostart = true;

bool writefinalscore;

void game_update()
{

	switch (state)
	{
		case Idle:
			/* If player starts hovering over start marker, set state to Starting
			 * and store the time -- this state (Idle) is for trial #1 only.
			 */

			Target.trial = 0;

			startCircle->SetPos(curtr.startx, curtr.starty);
			startCircle->On();
			targCircle->Off();

			velBar.On();

			photosensorCircle->On();

			if (!returntostart && CurTrial > 0)
			{
				//if we haven't yet gotten back to the start target yet
				startCircle->On();
				targCircle->Off();

				velBar.UpdateSpeed(LastPeakVel);
				velBar.On();
			}

			//shut off all traces, except the one currently requested
			for (int a = 0; a < NTRACES; a++)
				traces[a]->Off();

			
			//reset and shut off all paths
			for (int a = 0; a < NPATHS; a++)
			{
				barrierPaths[a].SetPathColor(grayColor);
				barrierPaths[a].Off();
				hitPath = false;
			}

			//reset and shut off all regions
			for (int a = 0; a < NREGIONS; a++)
			{
				barrierRegions[a].SetRegionColor(redColor);
				barrierRegions[a].Off();
				hitRegion = false;
			}
			

			if( (player->Distance(startCircle) <= CURSOR_RADIUS*1.5) && (CurTrial < NTRIALS) )
			{
				hoverTimer->Reset();
				trialTimer->Reset();
				
				if (curtr.trace >= 0)
				{
					traces[curtr.trace]->SetPos(curtr.startx,curtr.starty);
					traces[curtr.trace]->On();
				}

				trialtext.statusflagpath = Image::ImageText(trialtext.statusflagpath,"Hit Path: False","arial.ttf",15,textColor, drawstruc.drawsecondtext);
				trialtext.statusflagregion = Image::ImageText(trialtext.statusflagregion,"Hit Region: False","arial.ttf",15,textColor, drawstruc.drawsecondtext);
				trialtext.statusflagtgt = Image::ImageText(trialtext.statusflagtgt,"Hit Target: False","arial.ttf",15,textColor, drawstruc.drawsecondtext);


				std::cerr << "Leaving IDLE state." << std::endl;
				
				LastPeakVel = PeakVel;
				PeakVel = 0;

				returntostart = true;
				state = Starting;
			}
			break;
		case Starting: 
			/* If player stops hovering over start marker, set state to Idle and
			 * store the time  -- this is for new trials only!
			 */

			startCircle->On();
			startCircle->SetColor(startColor);
			targCircle->Off();

			velBar.UpdateSpeed(PeakVel);
			velBar.On();


			if (player->Distance(startCircle) > START_RADIUS)
			{
				state = Idle;
			}
			// If player hovers long enough, set state to Active
			else if (hoverTimer->Elapsed() >= curtr.iti)
			{
				Target.trial = CurTrial+1;

				targCircle->SetPos(curtr.startx+curtr.xpos, curtr.starty+curtr.ypos);
				targCircle->SetColor(targColor);
				targCircle->SetBorderColor(blkColor);
				targCircle->On();
				photosensorCircle->Off();

				//turn on the requested trace
				if (curtr.trace >= 0)
				{
					traces[curtr.trace]->SetPos(curtr.startx,curtr.starty);
					traces[curtr.trace]->On();
				}

				//turn on the requested path
				if (curtr.path >= 0)
				{
					barrierPaths[curtr.path].SetPathCenter(curtr.startx+curtr.xpos,curtr.starty+curtr.ypos);
					barrierPaths[curtr.path].On();
				}

				//turn on the requested region
				if (curtr.region >= 0)
				{
					barrierRegions[curtr.region].SetRegionCenter(curtr.startx,curtr.starty);
					barrierRegions[curtr.region].On();
				}

				trialTimer->Reset();

				startbeep->Play();

				reachedvelmin = false;
				reachedvelmax = false;
				mvtStarted = false;
				PeakVel = 0;

				mvmtEnded = false;
				hitTarget = false;

				std::cerr << "Leaving STARTING state." << std::endl;
				state = Active;
			}

			break;

		case Active:
			
			startCircle->On();
			targCircle->On();

			//detect the onset of hand movement, for calculating latency
			if (!mvtStarted && (player->Distance(startCircle) > START_RADIUS*1.5))
			{
				mvtStarted = true;
				movTimer->Reset();
				Target.lat = trialTimer->Elapsed();
			}

			//keep track of the maximum (peak) velocity -- this will be plotted in the feedback bar
			if ((player->GetVel() > PeakVel) && (player->Distance(startCircle) <= targCircle->Distance(startCircle)) )
				PeakVel = player->GetVel();

			//if uncommented, will update speed continuously during reach
			//velBar.UpdateSpeed(PeakVel);
			//velBar.On();

			//note if the velocity exceeded the minimum required velocity
			if (player->GetVel() > VELMIN)
			{
				//std::cerr << "Reached VelMin." << std::endl;
				reachedvelmin = true;
			}
			//note if the velocity exceeded the maximum  required velocity
			if (player->GetVel() > VELMAX)
			{
				//std::cerr << "Reached VelMax." << std::endl;
				reachedvelmax = true;
			}

			//check if the hand intersected the path
			if (curtr.path >= 0 && barrierPaths[curtr.path].PathCollision(player))
			{
				barrierPaths[curtr.path].SetPathColor(orangeColor);
				
				if (!hitPath)
					trialtext.statusflagpath = Image::ImageText(trialtext.statusflagpath,"Hit Path: True","arial.ttf",15,textColor, drawstruc.drawsecondtext);
				
				hitPath = true;
			}

			//check if the hand entered the region
			if (curtr.region >= 0 && barrierRegions[curtr.region].InRegion(player))
			{
				barrierRegions[curtr.region].SetRegionColor(orangeColor);
				
				if (!hitRegion)
					trialtext.statusflagregion = Image::ImageText(trialtext.statusflagregion,"Hit Region: True","arial.ttf",15,textColor, drawstruc.drawsecondtext);
				
				hitRegion = true;
			}

			
			//check if the player hit the target
			if (player->HitTarget(targCircle))
			{
				targCircle->SetColor(targHitColor);

				if (!hitTarget)
					trialtext.statusflagtgt = Image::ImageText(trialtext.statusflagtgt,"Hit Target: True","arial.ttf",15,textColor, drawstruc.drawsecondtext);

				hitTarget = true;
				
				
			}


			//detect movement offset
			if (!mvmtEnded && mvtStarted && (player->GetVel() < VEL_MVT_TH) && (movTimer->Elapsed()>200) && player->Distance(startCircle)>4*START_RADIUS)
				{
					mvmtEnded = true;
					hoverTimer->Reset();
					std::cerr << "Mvmt Ended: " << float(SDL_GetTicks()) << std::endl;
				}

			if ((player->GetVel() >= VEL_MVT_TH))
			{
				mvmtEnded = false;
				hoverTimer->Reset();
			}


			//if the trial duration is exceeded, the hand has stopped moving, or the hand has exceeded the target array, end the trial
			if ( (trialTimer->Elapsed() > MAX_TRIAL_DURATION) || (mvmtEnded && hoverTimer->Elapsed()>VEL_END_TIME ) || (player->Distance(startCircle) > targCircle->Distance(startCircle) ) )
			{
				LastPeakVel = PeakVel;
				PeakVel = 0;

				photosensorCircle->On();

				//std::cerr << "Target scored."  << std::endl;
				std::cerr << "Score Flags: " << (reachedvelmin ? "1" : "0")
					 	  << (reachedvelmax ? "1" : "0")
						  << (hitTarget ? "1" : "0")
						  << std::endl;

				if (reachedvelmin && !reachedvelmax && hitTarget && !hitPath && !hitRegion)  //if the velocity and latency criteria have been satisified and the target has been hit, score the trial
				{
					score++;     //target score
					scorebeep->Play();
				}
				else
				{
					if (hitTarget)  //if hit the target
					{
						//do nothing
					}
					if (!reachedvelmin || reachedvelmax)
					{
						errorbeep->Play();

						//note whether the movement during the trial satisified the velocity requirements
						if (!reachedvelmin)
							std::cerr << "Minimum Velocity not met."  << std::endl;
						if (reachedvelmax)
							std::cerr << "Maximum Velocity exceeded."  << std::endl;

						if (!hitTarget)
							targCircle->SetColor(blkColor);

					}

				}

				//go to ShowResult state
				trialTimer->Reset();// = SDL_GetTicks();
				state = ShowResult;

			}
			
			break;

		case ShowResult:

			returntostart = false;
			velBar.UpdateSpeed(LastPeakVel);
			velBar.On();


			if ( trialTimer->Elapsed() > HOLDTIME)
			{

				CurTrial++;
				std::stringstream texttn;
				//texttn << CurTrial+1;  //CurTrial starts from 0, so we add 1 for convention.
				//trialnum = Image::ImageText(trialnum,texttn.str().c_str(),"arial.ttf", 12,textColor, drawstruc.drawmaintext);

				texttn.str("");
				texttn << "Trial: " << CurTrial+1 << " of " << NTRIALS;
				trialtext.trialnum = Image::ImageText(trialtext.trialnum,texttn.str().c_str(),"arial.ttf",15,textColor, drawstruc.drawsecondtext);

				std::cerr << "Trial " << CurTrial << " ended at " << SDL_GetTicks() << std::endl;


				//if we have reached the end of the trial table, quit
				if (CurTrial >= NTRIALS)
				{
					std::cerr << "Leaving ACTIVE state to FINISHED state." << std::endl;
					trialTimer->Reset();
					writefinalscore = false;
					state = Finished;
				}
				else
				{
					hoverTimer->Reset();
					std::cerr << "Leaving ACTIVE state to Idle state." << std::endl;
					state = Idle;
				}
			}

			break;

		case Finished:
			// Trial table ended, wait for program to quit

			startCircle->Off();
			targCircle->Off();

			velBar.Off();

			for (int a = 0; a < NTRACES; a++)
				traces[a]->Off();
			
			for (int a = 0; a < NPATHS; a++)
				barrierPaths[a].Off();

			for (int a = 0; a < NREGIONS; a++)
				barrierRegions[a].Off();

			//provide the score at the end of the block.
			if (!writefinalscore)
			{
				std::stringstream scorestring;
				scorestring << "You earned " 
							<< score 
							<< " points.";
				text = Image::ImageText(text, scorestring.str().c_str(), "arial.ttf", 28, textColor, drawstruc.drawmaintext);

				trialtext.trialnum = Image::ImageText(trialtext.trialnum,"Trial: END","arial.ttf",15,textColor, drawstruc.drawsecondtext);

				writefinalscore = true;
			}
			text->On();

			if (trialTimer->Elapsed() > 5000)
				quit = true;


			break;

	}
}

