#include "Sound.h"

Sound::Sound(char* filePath)
{
	chunk = Mix_LoadWAV(filePath);
	status = -1;
}

Sound::~Sound()
{
	Mix_FreeChunk(chunk);
}

Mix_Chunk* Sound::GetChunk() const
{
	return chunk;
}

void Sound::Play(int loops)
{
	channel = Mix_PlayChannel(-1, chunk, loops);

	if (channel == -1)
	{
		status = 0;
		std::cerr << "Audio play error." << std::endl;
	}
	else
		IsPlaying(); //update the current status
}

int Sound::IsPlaying()
{

	if (channel > -1)
		status = Mix_Playing(channel);
	else //channel not valid, check if any channels are playing
	{
		status = Mix_Playing(-1);
		if (status > 0)
			status = 1;

	}

	return(status);
}