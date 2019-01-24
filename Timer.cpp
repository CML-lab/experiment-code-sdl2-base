#include "Timer.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "Sound.h"

/*
	To use the timer object for timed response:
	
	//Initialize the Timed-Response timer:
	TRtimer = new Timer(IBI,startbeep,3); //where IBI is the inter-beep interval, startbeep is the audio file of the tones, and 3 is the number of times to play the sound after now

	
	//Start the timed-response interval:
	startbeep->Play();		//play the tone to mark the start of the interval
	TRtimer->SetAlarmTime(IBI,3);  //set the alarm to ring 3 more times with a fixed inter-beep interval specified by IBI


	//Check on when the tones sound:
	TRtimer->CheckAlarm();  // check on the status of the timer; if the inter-beep interval has elapsed this will prompt the tone to sound. Tones will continue until the number of tones specified have been played
	stopped = TRtimer->CheckStopped(); // The timer will automatically stop after the last tone rings. This call will return TRUE if the timer has stopped.


	//The hand latency can be determined in one of two ways:
	if(movement_started)
	{
		lat = trialTimer->Elapsed()-(3*IBI);  //measure time elapsed from start of the timed-response interval to the start of movement and subtract out the 3 inter-beep intervals to get the hand latency. This assumes no weird lags in the inter-beep-interval timing.

		lat = int(SDL_GetTicks())-int(TRtimer->GetStopTime());  //measure difference between the current time and when the fourth tone actually occurred. Note, this only works if the fourth tone has occurred; if it hasn't happened yet then you have to store the time of movement onset and compute this after the fact.
	}
	

*/

Timer::Timer(void)
{
	startTime = SDL_GetTicks();
	alarmTime = 0;
	alarmSound = NULL;
	alarmSounded = 0;
	nAlarms = 0;
	alarmOn = 0;
	stopped = 1;
}


Timer::Timer(Uint32 alarmtime, Sound* alarm, int NAlarms)
{
	alarmSound = alarm;
	alarmTime = alarmtime;
	alarmSounded = NAlarms;
	nAlarms = NAlarms;
	alarmOn = 1;
	startTime = SDL_GetTicks();
	stopped = 1;
}


Timer::~Timer(void)
{
}


void Timer::Reset(void)
{
	startTime = SDL_GetTicks();
	stopped = 0;
}


Uint32 Timer::Elapsed(void)
{
	if(stopped)
	{
		return (stopTime-startTime);
	}
	else
	{
		return SDL_GetTicks()-startTime;
	}
}


void Timer::SetAlarmTime(Uint32 alarmT,int NAlarms)
{
	alarmTime = alarmT;
	alarmSounded = 0;
	nAlarms = NAlarms;
	Reset();
	stopped = 0;
}


Uint32 Timer::TimeToNextAlarm()
{
	return startTime+alarmTime*(alarmSounded+1) - SDL_GetTicks();
}


Uint32 Timer::RemainingTime()
{
	return startTime+alarmTime*nAlarms - SDL_GetTicks();
}


Uint32 Timer::GetStartTime()
{
	return startTime;
}


Uint32 Timer::GetStopTime()
{
	if (stopped == 1)
		return stopTime;
	else
		return NULL;
}


bool Timer::CheckAlarm()
{
	bool status = false;

	if(SDL_GetTicks()-startTime > alarmTime*(alarmSounded+1) && alarmOn && alarmSounded<nAlarms)
	{
		alarmSound->Play();
		alarmSounded++;
		status = true;
		//std::cerr << "Alarm: " << TRtimer->TimeToNextAlarm() << " :: " << TRtimer->RemainingTime() << std::endl;
	}
	else if(SDL_GetTicks()-startTime > alarmTime*(alarmSounded+1) && alarmSounded<nAlarms)
	{
		//if no sound file is specified and we are just using this as a countdown timer
		alarmSounded++;
		status = true;
	}

	if (alarmSounded>=nAlarms)
		Stop();

	return(status);
}


void Timer::Stop()
{
	stopTime = SDL_GetTicks();
	stopped = 1;
}


bool Timer::CheckStopped()
{
	return stopped;
}