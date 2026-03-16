#pragma once

#include <Windows.h>

class Timer
{
public:
	Timer();
	Timer(const Timer&);
	~Timer();

	bool Initialize();
	void Frame();

	double GetTime();

	void StartTimer();
	void StopTimer();
	int GetTiming();

private:
	double m_frequency;
	INT64 m_startTime;
	double m_frameTime;
	INT64 m_beginTime, m_endTime;
};