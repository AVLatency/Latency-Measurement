#pragma once
#include "AbstractOutput.h"
#include <string>

class ExternalMediaPlayerOutput : public AbstractOutput
{
public:
	ExternalMediaPlayerOutput(std::string fileName, bool loop);
	~ExternalMediaPlayerOutput();

	void StartPlayback();
	void StopPlayback();

private:
	std::atomic<bool> stopRequested = false;
	std::string fileName;
	bool loop;
};

