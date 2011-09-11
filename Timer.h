#ifndef TIMER_H
#define TIMER_H

void InitTimer();
void UpdateTimer();

real GetTimePassed();
real GetFps();
int GetFpsAverage();

// This will check the time when the function is called,
// it's usually better to only call UpdateTimer once per
// frame and then use GetTimePassed().
dword GetTimeStamp();

#endif // TIMER_H
