#pragma once
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include <thread>

class AdjustVolumeManager
{
public:
	WasapiInput* input = nullptr;
	WasapiOutput* output = nullptr;

	bool working = false;

	AdjustVolumeManager();
	~AdjustVolumeManager();
	void Tick();
	void Stop();

private:
	std::thread* inputThread = nullptr;
	std::thread* outputThread = nullptr;
};

