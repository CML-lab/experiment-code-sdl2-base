#include "Sound.h"

Sound::Sound(char* filePath)
{
	chunk = Mix_LoadWAV(filePath);
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
	Mix_PlayChannel(-1, chunk, loops);
}