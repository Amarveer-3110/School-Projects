#include <windows.h>
#include "Timer.h"


//-------------------------------------------------------------------------------------------
TimerType::TimerType()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&ticksPerSecond);
	secondsPerTick = 1.0 / (double)ticksPerSecond;

	QueryPerformanceCounter((LARGE_INTEGER*)&previousTime);
}

//-------------------------------------------------------------------------------------------
void TimerType::CheckTime()
{
	__int64 currTime;

	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	currentTime = currTime;

	// Time difference between this frame and the previous in seconds.
	deltaTime = (currentTime - previousTime)*secondsPerTick;

	framesPerSecond = (int) (ticksPerSecond / (currentTime - previousTime));

	// Prepare for next frame.
	previousTime = currentTime;

	// Force nonnegative.  
	if(deltaTime < 0.0)
		deltaTime = 0.0;

}

//-------------------------------------------------------------------------------------------
double TimerType::GetTimeDeltaTime()
{
	return deltaTime;
}