#ifndef TIMER_H
#define TIMER_H
#pragma once
#include "SDL.h"
#include "SDL_mixer.h"
#include "Sound.h"

class Timer
{
private:
	Uint32 startTime;
	Uint32 stopTime;
	Uint32 alarmTime;
	Sound* alarmSound;
	int nAlarms;
	int alarmSounded;
	bool alarmOn;
	bool stopped;

public:
	Timer(Uint32 alarmtime, Sound* alarm, int NAlarms);
	Timer(void);
	~Timer(void);

	void Reset();
	Uint32 Elapsed();
	Uint32 GetStartTime();
	Uint32 GetStopTime();
	void SetAlarmTime(Uint32 alarmT, int NAlarms);
	Uint32 TimeToNextAlarm();
	Uint32 RemainingTime();
	void Stop();
	bool CheckStopped();

	bool CheckAlarm();
};

#endif