#ifndef SOUND_H
#define SOUND_H
#pragma once

#include "SDL_mixer.h"
#include <iostream>

// Stores a sound for playback
class Sound
{
private:
	Mix_Chunk* chunk;
	int channel;
	int status;
public:
	// filePath is the path to the sound file to use to initialize the Sound
	Sound(char* filePath);
	~Sound();
	
	// Get the SDL_mixer chunk for this Sound
	Mix_Chunk* GetChunk() const;
	
	// Play the sound
	// loops is the number of times to replay the sound (default 0)
	void Play(int loops = 0);

	//check if the sound is still playing
	int IsPlaying();
};

#endif
