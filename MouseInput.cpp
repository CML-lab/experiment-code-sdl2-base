#include "config.h"
#include "MouseInput.h"

int MouseInput::GetFrame(TrackDATAFRAME DataMouseFrame[])
{
	DataMouseFrame[0].ValidInput = 1;

	DataMouseFrame[0].x = x;
	DataMouseFrame[0].y = y;
	DataMouseFrame[0].z = 0.0;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			DataMouseFrame[0].anglematrix[i][j] = 0.0f;

	DataMouseFrame[0].time = SDL_GetTicks();

	return(1);
}

void MouseInput::ProcessEvent(SDL_Event event)
{
	if (event.type == SDL_MOUSEMOTION)
	{
		// Convert position data to meters
		x = (GLfloat)event.motion.x * PHYSICAL_RATIO;
		y = (GLfloat)(SCREEN_HEIGHT - event.motion.y) * PHYSICAL_RATIO;
	}
}